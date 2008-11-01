/*
  This file is part of p4est.
  p4est is a C library to manage a parallel collection of quadtrees and/or
  octrees.

  Copyright (C) 2008 Carsten Burstedde, Lucas Wilcox.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef P4_TO_P8
#include <p4est.h>
#include <p4est_algorithms.h>
#else
#include <p8est.h>
#include <p8est_algorithms.h>
#endif

#ifndef P4_TO_P8
#define P4EST_CONN_SUFFIX "p4c"
#define P4EST_FOREST_SUFFIX "p4p"
#else
#define P4EST_CONN_SUFFIX "p8c"
#define P4EST_FOREST_SUFFIX "p8p"
#endif

int
main (int argc, char **argv)
{
  MPI_Comm            mpicomm;
  int                 mpiret;
  int                 mpisize, mpirank;
  p4est_connectivity_t *connectivity, *conn2;
  p4est_t            *p4est, *p4est2;

  /* initialize MPI */
  mpiret = MPI_Init (&argc, &argv);
  SC_CHECK_MPI (mpiret);
  mpicomm = MPI_COMM_WORLD;
  mpiret = MPI_Comm_size (mpicomm, &mpisize);
  SC_CHECK_MPI (mpiret);
  mpiret = MPI_Comm_rank (mpicomm, &mpirank);
  SC_CHECK_MPI (mpiret);

  sc_init (mpirank, sc_generic_abort, &mpicomm, NULL, SC_LP_DEFAULT);
  p4est_init (NULL, SC_LP_DEFAULT);

  /* create connectivity and p4est structures */
#ifndef P4_TO_P8
  connectivity = p4est_connectivity_new_star ();
#else
  connectivity = p8est_connectivity_new_rotcubes ();
#endif
  p4est = p4est_new (mpicomm, connectivity, 0, 0, NULL, NULL);

  /* save, synchronize, load connectivity and compare */
  if (mpirank == 0) {
    p4est_connectivity_save (P4EST_STRING "." P4EST_CONN_SUFFIX,
                             connectivity);
  }
  mpiret = MPI_Barrier (mpicomm);
  SC_CHECK_MPI (mpiret);
  conn2 = p4est_connectivity_load (P4EST_STRING "." P4EST_CONN_SUFFIX);
  SC_CHECK_ABORT (p4est_connectivity_is_equal (connectivity, conn2),
                  "load/save connectivity mismatch A");
  p4est_connectivity_destroy (conn2);

  /* save, synchronize, load p4est and compare */
  p4est_save (P4EST_STRING "." P4EST_FOREST_SUFFIX, p4est);
  p4est2 = p4est_load (P4EST_STRING "." P4EST_FOREST_SUFFIX, &conn2);
  SC_CHECK_ABORT (p4est_connectivity_is_equal (connectivity, conn2),
                  "load/save connectivity mismatch B");
#if 0
  SC_CHECK_ABORT (p4est_is_equal (p4est, p4est2), "load/save p4est mismatch");
  p4est_destroy (p4est2);
#endif
  p4est_connectivity_destroy (conn2);

  /* destroy data structures */
  p4est_destroy (p4est);
  p4est_connectivity_destroy (connectivity);

  /* clean up and exit */
  sc_finalize ();

  mpiret = MPI_Finalize ();
  SC_CHECK_MPI (mpiret);

  return 0;
}

/* EOF test_loadsave2.c */
