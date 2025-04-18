#include <cstdio>

std::vector<G4S_Particle> pvec, *pvec_ptr = &pvec;
TBranch *b = nullptr;

void dump_setup_branch(int event=0)
{
  if (b == nullptr) {
    b = T->GetBranch("p");
    b->SetAddress(&pvec_ptr);
  }
  b->GetEntry(event);
  printf("Loaded event %d, size of particle vector=%d, number of primaries=%d\n",
	       event, (int) pvec.size(), pvec[0].n_daughters());
}

void dump_json(std::vector<G4S_Particle> &out, const char *fname)
{
  TString json = TBufferJSON::ToJSON(&out);
  printf("Created JSON object, length=%d\n", (int) json.Length());
  FILE *fp = fopen(fname, "w");
  json.Puts(fp);
  fclose(fp);
  printf("Written JSON to '%s'\n", fname);
}

// -- primaries only

void dump_primaries(int event=0, const char *fname="primaries.json")
{
  dump_setup_branch(event);

  std::vector<G4S_Particle> out;
  // element 0 holds range of primaries
  int n_to_copy = 1 + pvec[0].n_daughters();
  out.reserve(n_to_copy);
  for (int i = 0; i < n_to_copy; ++i)
  {
    out.push_back( pvec[i] );
  }

  dump_json(out, fname);
}

// -- primaries and secondaries

void append_secondaries_of(int pi, std::vector<G4S_Particle> &out)
{
  // Assumes primaries have been copied to same positions as in pvec.
  // Appends secondaries of primary 'pi' to the end of 'out' vector.
  // Re-numbers daugter range for the primary.
  // In this case the parent indices of secondaries are correct.
  // Note -- for recursive dump one would need an additional index map.

  int dbeg = out.size();
  auto &p = out[pi];
  for (int i = p.m_daughters_begin; i != p.m_daughters_end; ++i) {
    out.push_back( pvec[i] );
  }
  p.m_daughters_begin = dbeg;
  p.m_daughters_end = out.size();
}

void dump_primaries_and_scondaries(int event=0, const char *fname="primaries-and-secondaries.json")
{
  dump_setup_branch(event);

  std::vector<G4S_Particle> out;
  // element 0 holds range of primaries
  int n_to_copy = 1 + pvec[0].n_daughters();
  out.reserve(n_to_copy);
  int n_daughters = 0;
  for (int i = 0; i < n_to_copy; ++i)
  {
    out.push_back( pvec[i] );
    n_daughters += pvec[i].n_daughters();
  }
  out.reserve(n_to_copy + n_daughters);
  for (int i = 1; i < n_to_copy; ++i)
  {
    append_secondaries_of(i, out);
  }

  dump_json(out, fname);
}

// -- dump all

void dump_all(int event=0, const char *fname="all-particles.json")
{
  dump_setup_branch(event);
  dump_json(pvec, fname);
}

// -- print content of initial particles for all events in the file

void print_primaries_for_all_events()
{
  int n_events = T->GetEntriesFast();
  for (int e = 0; e < n_events; ++e)
  {
    dump_setup_branch(e);
    int n_daughters = pvec[0].n_daughters();
    for (int p = 1; p <= n_daughters; ++p)
    {
      G4S_Particle &d = pvec[p];
      G4S_Particle::Vec4D &dxb = d.m_x_beg;
      printf("  %2d  %4d | %8.3f %8.3f %8.3f | %8.3f %6.3f\n",
             p, d.m_pdg, dxb.fX, dxb.fY, dxb.fZ, dxb.R(), dxb.Phi());
    }
    if (n_daughters == 2) {
      G4S_Particle::Vec4D x21 = pvec[2].m_x_beg - pvec[1].m_x_beg;
      G4S_Particle::Vec4D p21 = pvec[2].m_p_beg - pvec[1].m_p_beg;
      printf("  Dxy = %.4f,  De = %.4f\n", std::hypot(x21.fX, x21.fY), std::abs(p21.fT));
    }
  }
}
