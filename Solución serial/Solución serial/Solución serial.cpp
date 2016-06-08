// Solución serial.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <math.h>
#include "mpi.h"
int main()
{
	/* seudocigo
	1 get input data;
	2 for each timestep{
	3	if(timestep output) Print position and velocities of particles;
	4	for each particle q
	5		compute total force on q;
	6	for each particle q
	7		Compute position and velocity of q;
	8 }
	9 Print position and velocities of the particles;*/

	//numero de particulas: 400
	//numero de timesteps: 1000
	MPI_Init(NULL, NULL);
	int comm_sz, my_rank;
	
	const int X = 0, Y = 1, n=400, timestep=1000;
	const double G = 0.000000000663, delta=0.01;
	double pos[n][2], forces[n][2], masses[n], vel[n][2];
	double x_diff, y_diff, dist, dist_cubed;
	MPI_Datatype vect_mpi_t;
	MPI_Type_contiguous(2, MPI_DOUBLE, &vect_mpi_t);
	MPI_Type_commit(&vect_mpi_t);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	int loc_n = n / comm_sz;
	double loc_vel[2];
	if (my_rank == 0)
	{
		for (int i = 0; i < n; i++)
		{
			//leer masas, posicion  velocidad
			forces[i][0] = 0.0;
			forces[i][1] = 0.0;
			masses[i] = rand() % 50;
			pos[i][X] = (rand() % 200) - 100;
			pos[i][Y] = (rand() % 200) - 100;
			vel[i][X] = (double)((rand() % 10) - 5)/10;
			vel[i][Y] = (double)((rand() % 10) - 5) / 10;
		}
	}
	MPI_Bcast(masses, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(pos, n, vect_mpi_t, 0, MPI_COMM_WORLD);
	MPI_Bcast(vel, n, vect_mpi_t, 0, MPI_COMM_WORLD);
	//MPI_Scatter(vel, loc_n, vect_mpi_t, loc_vel, loc_n, vect_mpi_t, 0, MPI_COMM_WORLD);
	for (int i = 0; i < timestep; i++)
	{
		for (int q = my_rank*loc_n; q < my_rank*loc_n+loc_n; q++)
		{
			for (int k = my_rank*loc_n; k < my_rank*loc_n+loc_n && k!=q; k++)
			{
				x_diff = pos[q][X] - pos[k][X];
				y_diff = pos[q][Y] - pos[k][Y];
				dist = sqrt(x_diff*x_diff + y_diff*y_diff);
				dist_cubed = dist*dist*dist;
				forces[q][X] -= G*masses[q] * masses[k] / dist_cubed*x_diff;
				forces[q][Y] -= G*masses[q] * masses[k] / dist_cubed*y_diff;
			}
		}

		for (int q = my_rank*loc_n; q < my_rank*loc_n + loc_n; q++)
		{
			pos[q][X] += delta*vel[q][X];
			pos[q][Y] += delta*vel[q][Y];
			vel[q][X] += delta / masses[q] * forces[q][X];
			vel[q][Y] += delta / masses[q] * forces[q][Y];

		}


	}

	MPI_Allgather(MPI_IN_PLACE, loc_n, vect_mpi_t, pos, loc_n, vect_mpi_t, MPI_COMM_WORLD);

	MPI_Finalize();
    return 0;
}

