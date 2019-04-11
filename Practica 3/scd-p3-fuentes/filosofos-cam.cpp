// -----------------------------------------------------------------------------
// Alejandro Bonet Medina
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Se evita el interbloqueo mediante un proceso 'camarero' que da permiso
// a los filósofos para sentarse y levantarse de la mesa mediante
// espera selectiva.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos ,
   num_procesos_esperados = num_procesos +1,
   id_camarero=num_procesos;

// Etiquetas
const int
  etiq_sentarse=0,
  etiq_levantarse=1;
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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_procesos, //id. tenedor izq.
      id_ten_der = (id+num_procesos-1) % num_procesos, //id. tenedor der.
      peticion;
  while ( true )
  {
    //1.sentarse
    cout <<"Filósofo " <<id << " solicita permiso para sentarse."<<endl;
    MPI_Ssend(&peticion,1,MPI_INT,id_camarero,etiq_sentarse,MPI_COMM_WORLD);

    //2.Tomar tenedores
    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    MPI_Ssend(&peticion,1,MPI_INT,id_ten_izq,0,MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    MPI_Ssend(&peticion,1,MPI_INT,id_ten_der,0,MPI_COMM_WORLD);

    //3.Comer
    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    //4.Soltar tenedores
    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    MPI_Ssend(&peticion,1,MPI_INT,id_ten_izq,0,MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    MPI_Ssend(&peticion,1,MPI_INT,id_ten_der,0,MPI_COMM_WORLD);

    //5.Solicitar levantarse de la mesa
    cout <<"Filósofo " <<id << " solicita permiso para levantarse."<<endl;
    MPI_Ssend(&peticion,1,MPI_INT,id_camarero,etiq_levantarse,MPI_COMM_WORLD);

    //6.pensar
    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     MPI_Recv(&valor,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&estado);
     id_filosofo=estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     MPI_Recv(&valor,1,MPI_INT,id_filosofo,0,MPI_COMM_WORLD,&estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------
void funcion_camarero(){
  int s=0,
  peticion,
  id_filosofo,
  etiqueta_aceptable;
  MPI_Status estado;

 while(true){
   if(s<num_filosofos-1)
    etiqueta_aceptable=MPI_ANY_TAG;
    else
    etiqueta_aceptable=etiq_levantarse;

    MPI_Recv(&peticion,1,MPI_INT,MPI_ANY_SOURCE,etiqueta_aceptable,MPI_COMM_WORLD,&estado);
    id_filosofo=estado.MPI_SOURCE;

    switch (estado.MPI_TAG){
      case etiq_levantarse:
      cout<<"Filosofo "<<id_filosofo<<" se levanta de la mesa"<<endl;
      s--;
      break;
      case etiq_sentarse:
      cout<<"Filosofo "<<id_filosofo<<" se sienta de la mesa"<<endl;
      s++;
    }
    cout<<"Actualmente hay "<<s<<" filosofos sentados"<<endl;
 }
}
//-------------------------------------------------------------------------
int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos_esperados == num_procesos_actual )
   {
     if(id_propio==id_camarero)
         funcion_camarero();
      else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------