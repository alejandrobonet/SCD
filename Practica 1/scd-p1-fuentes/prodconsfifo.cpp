#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40 ,   // número de items
	       tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int buffer[tam_vec];          //Buffer donde se almacenan los datos producidos
int primera_ocupada=0;          //Indice en el vector de la primera celda ocupada.Se incrementa al leer
int primera_libre=0;		//Indice en el vector de la primera celda libre.Se incrementa al escribir
Semaphore producir = tam_vec; //semaforo que controla el productor tam_vector si puede leer 0 sino (k+ #E+ #L)
Semaphore consumir = 0;       //Semaforo que controla el cosnumidor 1 si puede escribir, 0 sino ( #E - #L )
Semaphore exclusion = 1;

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

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      sem_wait(producir); //Se decrementa el semaforo producir dejandolo en espera para que no se produca un dato hasta que se haya consumido otro
      

      buffer[primera_libre]=dato;
      primera_libre=(primera_libre+1) % tam_vec;

      cout<<"Valor introducido en el buffer"<<endl;
      sem_signal(consumir); //Se incrementa el semaforo consumir una vez se ha producido el dato para liberar el proceso que consume un dato
      
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      sem_wait(consumir); //Se decrementa el semaforo consumir dejandolo en espera para que no se consuman mas datos hasta que se produzca otro
      
      int dato = buffer[primera_ocupada];
      primera_ocupada=(primera_ocupada +1) % tam_vec;

      cout<<"Dato leido del buffer"<<endl;
      sem_signal(producir) ; //Se incrementa el semaforo producir una vez se ha consumido el dato para liberar el proceso que produce un dato
      
      consumir_dato( dato ) ;
      

    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();
}
