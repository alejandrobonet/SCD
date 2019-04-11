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
	
mutex mtx;
const int N=5;

class Barberia : public HoareMonitor
{
 private:
 CondVar durmiendo;
 CondVar silla;
 CondVar sala;
 
 public:                    // constructor y métodos públicos
   Barberia() ;           // constructor
   void cortarPelo(int i);
   void siguienteCliente();
   void finCliente();
   
} ;

 Barberia::Barberia(){
 durmiendo=newCondVar();
 silla = newCondVar();
 sala = newCondVar();

}


void Barberia::siguienteCliente(){

 if(sala.empty()){
  mtx.lock();
  cout<<"El barbero se duerme porque no hay clientes."<<endl;
 mtx.unlock();
  durmiendo.wait();
 }
  mtx.lock();
  cout<<"El barbero llama a un cliente"<<endl;
 mtx.unlock();
  sala.signal();
 
}

void Barberia::finCliente(){
 silla.signal();
}

void Barberia::cortarPelo(int i){
 mtx.lock();
 cout<<"El cliente "<<i<<" entra en la barberia"<<endl;
 mtx.unlock();
 if(!durmiendo.empty()){
  mtx.lock();
  cout<<"El cliente "<<i<<" despierta al barbero"<<endl;
 mtx.unlock();
  durmiendo.signal();
 }
 if(!silla.empty()){
  mtx.lock();
  cout<<"El cliente "<<i<<" espera en la sala"<<endl;
   mtx.unlock();
  sala.wait();
 }
  mtx.lock();
 cout<<"El cliente "<<i<< " se va a la silla a pelarse"<<endl;
 mtx.unlock();
 silla.wait();
  mtx.lock();

 cout<<"El cliente "<<i<<" se marcha de la barberia"<<endl;
 mtx.unlock();
 
 
 

}

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


void esperarFueraBarberia(int cliente){
    chrono::milliseconds esperarFuera( aleatorio<20,200>() );

 mtx.lock();
    cout << "Cliente " << cliente 
          << " espera fuera de la barberia  (" << esperarFuera.count() << " milisegundos)" << endl;
    mtx.unlock();
   
   this_thread::sleep_for( esperarFuera );
 mtx.lock();
  cout << "Cliente " << cliente << " termina de esperar fuera." << endl;
 mtx.unlock();
}
void cortarPeloACliente(){
    chrono::milliseconds corta( aleatorio<20,200>() );

  this_thread::sleep_for( corta );
 mtx.lock();
    cout << "Barbero pela al cliente (" << corta.count() << " milisegundos)" << endl;
    mtx.unlock();
}

void funcion_hebra_cliente(MRef<Barberia> barberia,int cliente){

 while(true){
  barberia->cortarPelo(cliente);
  esperarFueraBarberia(cliente);
}


}	

void funcion_hebra_barbero(MRef<Barberia> barberia){

 while(true){
  barberia->siguienteCliente();
  cortarPeloACliente();
  barberia->finCliente();
 }
}

int main(){


   cout << "--------------------------------------------------------" << endl
        << "Problema de la barberia SU." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
 
   // declarar hebras y ponerlas en marcha

   MRef<Barberia> barberia = Create<Barberia>( );
thread barbero ( funcion_hebra_barbero,barberia),
          clientes[N];

 for(int i = 0;i<N;i++)
  clientes[i] = thread (funcion_hebra_cliente,barberia,i);

barbero.join();

for(int i=0;i<N;i++)
  clientes[i].join();

}
