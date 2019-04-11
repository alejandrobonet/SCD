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

const int N=3;
mutex mtx;	
class Estanco : public HoareMonitor
{
 private:
 
 int mostrador; //=-1 si esta vacio, =1 ingrediente 1, =2 ingrediente 2, =3 ingrediente 3

 CondVar vacio;
 
 CondVar ingrediente_dispo[N];            

 public:                    // constructor y métodos públicos
   Estanco() ;           // constructor
   void obtenerIngrediente(int i);
   void ponerIngrediente(int i);
   void esperarRecogidaIngrediente();

} ;
Estanco::Estanco()
{
   vacio = newCondVar();
   mostrador=-1;
   for (int i=0;i<N;i++)
   ingrediente_dispo[i] = newCondVar();
}

void Estanco::ponerIngrediente(int i){
 mostrador=i;
 cout<<"Ingrediente "<<i<<" se ha puesto en el mostrador."<<endl;
 ingrediente_dispo[i].signal();
}

void Estanco::obtenerIngrediente(int i){
 if(mostrador!=i)
  ingrediente_dispo[i].wait();

 assert(mostrador==i);
  mostrador=-1;
  vacio.signal();

}
void Estanco::esperarRecogidaIngrediente(){
 if(mostrador!=-1)
  vacio.wait();
}
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

int Producir(){
 //espera con retardo aleatorio
 this_thread::sleep_for(chrono::milliseconds(aleatorio<20,200>() ));
 return aleatorio<0,N-1>();
}
//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> estanco)
{
 while(true){
  int i = Producir();
  estanco->ponerIngrediente(i);
  estanco->esperarRecogidaIngrediente();
 }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador( MRef<Estanco> estanco,int num_fumador )
{
 
   while( true )
   {
    estanco->obtenerIngrediente(num_fumador);
    fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{

   cout << "--------------------------------------------------------" << endl
        << "Problema de los fumadores SU." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
 
   // declarar hebras y ponerlas en marcha

   MRef<Estanco> estanco = Create<Estanco>( );

   thread estanquero ( funcion_hebra_estanquero,estanco),
          fumadores[N];



   for(int i = 0; i < N; i++)
    fumadores[i] = thread ( funcion_hebra_fumador,estanco, i );

//No es necesario esperar a las hebras ya que se realiza un bucle infinito 
  for(int i = 0; i < N; i++)
   fumadores[i].join();

  
  estanquero.join();
}
