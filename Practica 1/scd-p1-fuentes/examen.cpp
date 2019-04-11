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
const int ITER = 10;
Semaphore mostr_vacio = 1, //1 si el mostrador está vacio 0 si está ocupado
	  puede_suministrar = 1,
	  suministra=0,
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
  sem_wait(suministra);
  for(int j=0;j<ITER;j++){
  int i = Producir();
  
  sem_wait( mostr_vacio );
  cout<<"Disponible ingrediente: "<<(i == 0 ? "papel" : "tabaco ")<<" ("<<i<<") "<<endl;
  sem_signal(ingr_disp[i]);
 }
 sem_signal(puede_suministrar);
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

void funcion_hebra_suministradora(){

 while(true){
  sem_wait(puede_suministrar);
  cout<<"\n*** Ha llegado un nuevo lote de suministros para el estanquero. ***\n"<<endl;
  //simula el acto de suministrar
  this_thread::sleep_for (chrono::milliseconds(aleatorio<20,200>()));
  sem_signal(suministra);
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
          fumadores[N],
	  hebra_suministradora(funcion_hebra_suministradora);
  
   for(int i = 0; i < N; i++)
    fumadores[i] = thread ( funcion_hebra_fumador, i );

//No es necesario esperar a las hebras ya que se realiza un bucle infinito 
 // for(int i = 0; i < N; i++)
  //fumadores[i].join();

  //Es necesario esperar al menos a una pues sino la hebra main finaliza produciendo error
  estanquero.join();
//hebra_suministradora.join();
}
