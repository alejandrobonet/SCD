#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int N=3;
Semaphore mostr_vacio = 1, //1 si el mostrador está vacio 0 si está ocupado
          ingr_disp[N] = {0,0,0}; //Si el ingrediente esta disponible, 0 si no




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

void funcion_hebra_estanquero(  )
{
 while(true){
  int i = Producir();
  assert(i<N);
  sem_wait( mostr_vacio );
  cout<<"El estanquero deja el ingrediente "<<i<<" en el mostrador "<<endl;
  sem_signal(ingr_disp[i]);
 }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
    sem_wait( ingr_disp[num_fumador] );
    cout<<"El fumador "<<num_fumador<<" retira su ingrediente."<<endl;
    sem_signal ( mostr_vacio );
    fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{

   cout << "--------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
 
   // declarar hebras y ponerlas en marcha
   thread estanquero ( funcion_hebra_estanquero ),
          fumadores[N];
  
   for(int i = 0; i < N; i++)
    fumadores[i] = thread ( funcion_hebra_fumador, i );

//No es necesario esperar a las hebras ya que se realiza un bucle infinito 
  for(int i = 0; i < N; i++)
   fumadores[i].join();

  
  estanquero.join();
}
