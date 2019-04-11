// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons_2su.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del productor/consumidor, con un único productor y un único consumidor.
// Opcion LIFO (stack)
//
// Historial:
// Creado en Julio de 2017
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

constexpr int
   num_items  = 40,      // número de items a producir/consumir
   nc=5,
   np=4;

int producidos[np],
    consumidos[nc];

mutex
   mtx ;   
              // mutex de escritura en pantalla
unsigned
   
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items]; // contadores de verificación: consumidos

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int hebra)
{

   int contador=producidos[hebra]+hebra*(num_items/np)	;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "producido: " << contador <<" por hebra "<<hebra<< endl << flush ;
   mtx.unlock();
   cont_prod[contador] ++ ;
   producidos[hebra]++;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato,int hebra)
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }

   cont_cons[dato] ++ ;
   consumidos[hebra]++;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << " por hebra "<<hebra<<endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, un prod. y un cons.

class ProdCons2SU : public HoareMonitor
{
 private:
 static const int           // constantes:
   num_celdas_total = 10;   //  núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//  buffer de tamaño fijo, con los datos
   primera_libre ;          //  indice de celda de la próxima inserción

 CondVar ocupadas,
	 libres;               

 public:                    // constructor y métodos públicos
   ProdCons2SU(  ) ;           // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdCons2SU::ProdCons2SU()
{
   primera_libre = 0 ;
   libres = newCondVar();
   ocupadas = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons2SU::leer(  )
{

   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   if ( primera_libre == 0 )
      ocupadas.wait();

   // hacer la operación de lectura, actualizando estado del monitor
   assert( 0 < primera_libre  );
   primera_libre-- ;
   const int valor = buffer[primera_libre] ;


   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdCons2SU::escribir( int valor )
{
   // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda();

   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   if ( primera_libre == num_celdas_total )
      libres.wait();

   //cout << "escribir: ocup == " << num_celdas_ocupadas << ", total == " << num_celdas_total << endl ;
   assert( primera_libre < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre++ ;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdCons2SU> monitor ,int hebra )
{
   for( unsigned i = 0 ; i < num_items/np ; i++ )
   {
      int valor = producir_dato(hebra) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora(MRef<ProdCons2SU> monitor,int hebra )
{
   for( unsigned i = 0 ; i < num_items/nc ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor,hebra ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (1 prod/cons, Monitor SC, buffer LIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;
   thread productores[np],
	  consumidores[nc];

 MRef<ProdCons2SU> monitor = Create<ProdCons2SU>();

 for(int i=0;i<np;i++){
  producidos[i]=0;
  productores[i] = thread( funcion_hebra_productora, monitor,i );
 }

 for(int i=0;i<nc;i++){
  consumidos[i]=0;
  consumidores[i]=thread( funcion_hebra_consumidora, monitor,i );
 }

 for(int i=0;i<np;i++)
    productores[i].join();

for(int i=0;i<nc;i++)
  consumidores[i].join();


   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores() ;
}
