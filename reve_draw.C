// assumes available through loader.C
// #include "G4SnitchDataFormat.h"

#include "TParticle.h"
#include <ROOT/REveElement.hxx>
#include <ROOT/REveScene.hxx>
#include <ROOT/REveManager.hxx>
#include <ROOT/REveTrack.hxx>
#include <ROOT/REveTrackPropagator.hxx>

#include "TMath.h"
#include <cstdio>

namespace REX = ROOT::Experimental;

using namespace REX;

REveTrackPropagator* g_prop = nullptr;

//==============================================================================

class CmsMagField: public REveMagField
{
   bool m_magnetIsOn;
   bool m_reverse;
   bool m_simpleModel;

public:
   CmsMagField():
      m_magnetIsOn(true),
      m_reverse(false),
      m_simpleModel(true){}

   ~CmsMagField() override{}

   void setMagnetState( bool state )
   {
      if (state != m_magnetIsOn)
      {
         if ( state )
            std::cout << "Magnet state is changed to ON" << std::endl;
         else
            std::cout << "Magnet state is changed to OFF" << std::endl;
      }
      m_magnetIsOn = state;
   }

   bool isMagnetOn() const               { return m_magnetIsOn;}
   void setReverseState(bool state)      { m_reverse = state; }
   bool isReverse() const                { return m_reverse;}
   void setSimpleModel(bool simpleModel) { m_simpleModel = simpleModel; }
   bool isSimpleModel() const            { return m_simpleModel;}

   Double_t GetMaxFieldMag() const override { return m_magnetIsOn ? 3.8 : 0.0; }

   REveVectorD GetField(Double_t x, Double_t y, Double_t z) const override
   {
      double R = sqrt(x*x+y*y);
      double field = m_reverse ? -GetMaxFieldMag() : GetMaxFieldMag();
      //barrel
      if ( TMath::Abs(z) < 724 )
      {
         //inside solenoid
         if ( R < 300) return REveVectorD(0,0,field);
         // outside solenoid
         if ( m_simpleModel ||
              ( R>461.0 && R<490.5 ) ||
              ( R>534.5 && R<597.5 ) ||
              ( R>637.0 && R<700.0 ) )
            return REveVectorD(0,0,-field/3.8*1.2);

      } else {
         // endcaps
         if (m_simpleModel)
         {
            if ( R < 50 ) return REveVectorD(0,0,field);
            if ( z > 0 )
               return REveVectorD(x/R*field/3.8*2.0, y/R*field/3.8*2.0, 0);
            else
               return REveVectorD(-x/R*field/3.8*2.0, -y/R*field/3.8*2.0, 0);
         }
         // proper model
         if ( ( TMath::Abs(z)>724 && TMath::Abs(z)<786  ) ||
              ( TMath::Abs(z)>850 && TMath::Abs(z)<910  ) ||
              ( TMath::Abs(z)>975 && TMath::Abs(z)<1003 ) )
         {
            if ( z > 0 )
               return REveVectorD(x/R*field/3.8*2.0, y/R*field/3.8*2.0, 0);
            else
               return REveVectorD(-x/R*field/3.8*2.0, -y/R*field/3.8*2.0, 0);
         }
      }
      return REveVectorD(0,0,0);
   }
};

void setup_prop_for_cms(REveTrackPropagator* prop, bool simple_field)
{
   auto mf = new CmsMagField;
   // mf->setReverseState(true); // ??? was needed for TEve (wrong field sign inherited from ALICE sim)
   if (!simple_field)
      mf->setSimpleModel(false);

   prop->SetMagFieldObj(mf);
   prop->SetMaxR(1000);
   prop->SetMaxZ(1000);
   prop->SetRnrReferences(kTRUE);
   prop->SetRnrDaughters(kTRUE);
   prop->SetRnrDecay(kTRUE);
   prop->RefPMAtt().SetMarkerStyle(4);
}

//==============================================================================

const bool primary_verbose = false;
const double pt_min = 0.5, pt_max = 500.0;
const double pt_min_sec = 0.01;

int total_tracks = 0;

bool primary_select(int i) {
   auto &p = pvec[i];
   double pt = p.m_p_beg.Perp();
   bool pass = pt >= pt_min && pt <= pt_max;
   if (primary_verbose && pass)
     printf("%3d: primary pass pT=%.2f\n", i, pt);
   return pass;
}

bool secondary_select(int i) {
   auto &p = pvec[i];
   double pt = p.m_p_beg.Perp();
   return pt >= pt_min_sec;
}


Color_t color_from_charge(int charge) {
   if (charge < 0) return kBlue;
   else if (charge > 0) return kRed;
   else return kGreen;
}

void add_track(int i, int level, int max_level, REveElement *list, REveTrackPropagator *prop)
{
   --max_level;
   ++level;
   ++total_tracks;

   auto &p = pvec[i];
   auto rc = new REveRecTrackD();
   rc->fV = p.m_x_beg;
   rc->fP = p.m_p_beg;
   rc->fSign = p.m_charge;

   auto track = new REveTrack(rc, prop);

   track->SetLineColor(color_from_charge(p.m_charge));

   for (int di = p.m_daughters_begin; di != p.m_daughters_end; ++di) {
      auto &d = pvec[di];
      track->AddPathMark(REvePathMarkD(REvePathMarkD::kDaughter, d.m_x_beg, d.m_p_beg));
      if (max_level && secondary_select(di))
         add_track(di, level, max_level, track, prop);
   }

   // track->AddPathMark(REvePathMarkD(REvePathMarkD::kReference, p.m_x_end, p.m_p_end));
   track->AddPathMark(REvePathMarkD(REvePathMarkD::kDecay, p.m_x_end));

   track->SetTitle(TString::Format("Particle %d, pdg=%d chg=%d pT=%.2f level=%d",
                   i, p.m_pdg, p.m_charge, p.m_p_beg.Perp(), level).Data());
   track->SetPickable(true);

   list->AddElement(track);
}

//==============================================================================

const bool isRungeKutta = true;

void reve_draw(int max_level=3)
{
   auto eveMng = REveManager::Create();

   auto list = new REveTrackList();
   auto prop = g_prop = list->GetPropagator();
   prop->SetFitDaughters(kFALSE);

   eveMng->GetEventScene()->AddElement(list);

   if (isRungeKutta) {
      prop->SetStepper(REveTrackPropagator::kRungeKutta);
      list->SetName("RK Propagator");
      list->SetLineColor(kMagenta);
   } else {
      list->SetName("Helix Propagator");
      list->SetLineColor(kCyan);
   }

   // ----------------------------------------

   dump_setup_branch();

   int n_prim = pvec[0].n_daughters();
   printf("n_primaries = %d\n", n_prim);

   int n_match = 0;
   for (int i = 1; i <= n_prim; ++i)
   {
      if (primary_select(i)) {
         ++n_match;
         add_track(i, 0, max_level, list, prop);
      }
   }
   printf("There are %d primaries between pT %.2f and %.2f, total_tracks=%d\n",
          n_match, pt_min, pt_max, total_tracks);

   list->MakeTracks();

   // ----------------------------------------

   eveMng->Show();
}
