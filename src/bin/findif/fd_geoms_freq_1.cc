/*! \file
    \ingroup OPTKING
    \brief fd_geoms_freq_1(): returns geometries necessary for finite-difference
     computation of frequencies from gradients; puts undisplaced geometry last in list
*/

#include "findif.h"

namespace psi { namespace findif {

std::vector< boost::shared_ptr<Matrix> > fd_geoms_freq_1(Options &options) {

  fprintf(outfile,"\tUsing finite-differences of gradients to determine vibrational frequencies");
  fprintf(outfile,"\t and normal modes.\n  ** Results are valid only at stationary points! **\n");

  int pts = options.get_int("POINTS");
  fprintf(outfile,"\tGenerating geometries for use with %d-point formula.\n", pts);
  if (pts != 3 && pts != 5)
    throw PsiException("FINDIF: Invalid number of points!",__FILE__,__LINE__);

  double disp_size = options.get_double("DISP_SIZE");
  fprintf(outfile,"\tDisplacement size will be %6.2e.\n", disp_size);

  const boost::shared_ptr<Molecule> mol = psi::Process::environment.molecule();

  // Get SALCS from libmints: all modes with rotations and translations projected out
  boost::shared_ptr<MatrixFactory> fact;
  CdSalcList salc_list(mol, fact);

  int Natom = mol->natom();
  fprintf(outfile,"\tNumber of atoms is %d.\n", Natom);

  int Nirrep = salc_list.nirrep();
  fprintf(outfile,"\tNumber of irreps is %d.\n", Nirrep);

  int Nsalc_all = salc_list.ncd();
  fprintf(outfile,"\tTotal Number of SALCS is %d.\n", Nsalc_all);

  if (options.get_int("PRINT") > 2)
    for (int i=0; i<Nsalc_all; ++i)
      salc_list[i].print();

  // build vectors that list indices of salcs for each irrep
  std::vector< std::vector<int> > salcs_pi; // salcs per irrep
  for (int h=0; h<Nirrep; ++h)
    salcs_pi.push_back( std::vector<int>() );
  for (int i=0; i<Nsalc_all; ++i)
    salcs_pi[salc_list[i].irrep()].push_back(i);

  fprintf(outfile,"\tIndex of salcs per irrep:\n");
  for (int h=0; h<Nirrep; ++h) {
    fprintf(outfile, "\t %d : ", h+1);
    for (int i=0; i<salcs_pi[h].size(); ++i)
      fprintf(outfile," %d ", salcs_pi[h][i]);
    fprintf(outfile, "\n");
  }

  // From now on in code, salcs_pi establishes the canonical order of SALCs and displacements
  fprintf(outfile,"\tNumber of SALC's per irrep:\n");
  for (int h=0; h<Nirrep; ++h)
    fprintf(outfile,"\t\t Irrep %d: %d\n", h+1, (int) salcs_pi[h].size());

  // Determine number of displacements
  std::vector<int> Ndisp_pi (Nirrep);

  // displacements for symmetric coordinates
  if (pts == 3)
    Ndisp_pi[0] = 2 * salcs_pi[0].size();
  else if (pts == 5)
    Ndisp_pi[0] = 4 * salcs_pi[0].size();

  // displacements for asymmetric coordinates
  for (int h=1; h<Nirrep; ++h) {
    if (pts == 3)
      Ndisp_pi[h] = salcs_pi[h].size();
    else if (pts == 5)
      Ndisp_pi[h] = 2* salcs_pi[h].size();
  }

  int Ndisp_all = 0;
  for (int h=0; h<Nirrep; ++h)
    Ndisp_all += Ndisp_pi[h];

  fprintf(outfile,"\tTotal number of displacements is %d.\n", Ndisp_all);
  fprintf(outfile,"\tNumber of displacements per irrep:\n");
  for (int h=0; h<Nirrep; ++h)
    fprintf(outfile,"\t\t Irrep %d: %d\n", h+1, Ndisp_pi[h]);

  // Get reference geometry
  Matrix ref_geom_temp = mol->geometry();
  SharedMatrix ref_geom(ref_geom_temp.clone());
  ref_geom->set_name("Reference geometry");

  // to be returned and converted into "matrix_vector" list in python
  std::vector< boost::shared_ptr<Matrix> > disp_geoms;

  for (int h=0; h<Nirrep; ++h) { // loop over irreps

    for (int i=0; i<salcs_pi[h].size(); ++i) { // loop over salcs of this irrep
      int salc_i = salcs_pi[h][i];   // index in cdsalc of this salc

      if (h == 0) { // symmetric displacements
        if (pts == 3) {
          boost::shared_ptr<Matrix> geom1(ref_geom->clone());
          displace_cart(geom1, salc_list, salc_i, -1, disp_size);
          disp_geoms.push_back(geom1);

          boost::shared_ptr<Matrix> geom2(ref_geom->clone());
          displace_cart(geom2, salc_list, salc_i, +1, disp_size);
          disp_geoms.push_back(geom2);
        }
        else if (pts == 5) {
          boost::shared_ptr<Matrix> geom1(ref_geom->clone());
          displace_cart(geom1, salc_list, salc_i, -2, disp_size);
          disp_geoms.push_back(geom1);

          boost::shared_ptr<Matrix> geom2(ref_geom->clone());
          displace_cart(geom2, salc_list, salc_i, -1, disp_size);
          disp_geoms.push_back(geom2);

          boost::shared_ptr<Matrix> geom3(ref_geom->clone());
          displace_cart(geom3, salc_list, salc_i, +1, disp_size);
          disp_geoms.push_back(geom3);

          boost::shared_ptr<Matrix> geom4(ref_geom->clone());
          displace_cart(geom4, salc_list, salc_i, +2, disp_size);
          disp_geoms.push_back(geom4);
        }
      }
      else { // h != 0; assymmetric displacements
        if (pts == 3) {
          boost::shared_ptr<Matrix> geom1(ref_geom->clone());
          displace_cart(geom1, salc_list, salc_i, -1, disp_size);
          disp_geoms.push_back(geom1);
        }
        else if (pts == 5) {
          boost::shared_ptr<Matrix> geom1(ref_geom->clone());
          displace_cart(geom1, salc_list, salc_i, -2, disp_size);
          disp_geoms.push_back(geom1);

          boost::shared_ptr<Matrix> geom2(ref_geom->clone());
          displace_cart(geom2, salc_list, salc_i, -1, disp_size);
          disp_geoms.push_back(geom2);
        }
      }
    } // i, salcs of this irrep

  } // h, irreps

  // put reference geometry list in list - though we don't need its gradient!
  disp_geoms.push_back(ref_geom);

  if (options.get_int("PRINT") > 2)
    for (int i=0; i<disp_geoms.size(); ++i)
      disp_geoms[i]->print();

  return disp_geoms;
}

}}
