// -*- C++ -*-
//
// Package:    EleAnalyzer
// Class:      EleAnalyzer
//
/**\class EleAnalyzer EleAnalyzer.cc NWU/EleAnalyzer/src/EleAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Andrey Pozdnyakov
//         Created:  Thu Sep 19 15:13:20 CDT 2013

#include <memory>
#include <iostream>
#include <fstream>
#include <string>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"


#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrackFwd.h"

#include "TTree.h"
#include "TLorentzVector.h"
#include "HistManager.h"

using namespace std;
using namespace edm;

class EleAnalyzer : public edm::EDAnalyzer {
   public:
      explicit EleAnalyzer(const edm::ParameterSet&);
      ~EleAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

      // ----------member data ---------------------------

  HistManager *hists;
  TFile *outFile;

  //TTree *ktree;

  TLorentzVector *p_lminus, *p_lplus, *p_gamma;

};
EleAnalyzer::EleAnalyzer(const edm::ParameterSet& iConfig)
{}

EleAnalyzer::~EleAnalyzer()
{
  delete hists;
  delete outFile;
}

void EleAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  if(iEvent.isRealData())
    exit(0);

  //cout<<"event  "<<iEvent.id().event()<<endl;

  Handle<reco::GenParticleCollection> genParticleColl;
  iEvent.getByLabel("genParticles", genParticleColl);

  vector<TLorentzVector> mu_plus, mu_minus, el_plus, el_minus;
  TLorentzVector gen_l1, gen_l2,gen_gamma;
  TLorentzVector gen_lPt1, gen_lPt2;

  Int_t zStarId = 23; //normal
  //Int_t zStarId = 3000001; //a hack

  UInt_t nel = 0;
  for (reco::GenParticleCollection::const_iterator it = genParticleColl->begin(); it != genParticleColl->end(); ++it) {
    if (abs(it->pdgId()) == 11 &&   it->mother()->pdgId() == zStarId ) //for pythia6
      {
        //cout<<"its an electrobn, it's status = "<<it->status()<<";\t  it's mother is: "<<it->mother()->pdgId()<<endl;
        if (it->pdgId()==11)
          {
            gen_l1 = TLorentzVector(it->px(), it->py(), it->pz(), it->energy());
            el_minus.push_back(TLorentzVector(it->px(), it->py(), it->pz(), it->energy()));
          }
        if (it->pdgId()==-11)
          {
            gen_l2 = TLorentzVector(it->px(), it->py(), it->pz(), it->energy());
            el_plus.push_back(TLorentzVector(it->px(), it->py(), it->pz(), it->energy()));
          }
        hists->fill1DHist(it->pdgId(),"pdg_el","Number of electrons from a Z", 40,-20,20,  1, "");
      }

    if (it->pdgId() == 22 && (it->mother()->pdgId() == 25 || it->mother()->pdgId() == zStarId)) //gamma from the higgs or a Z!
      {
        //cout<<"its a gamma, it's status = "<<it->status()<<";\t  it's mother is: "<<it->mother()->pdgId()<<endl;
        hists->fill1DHist(it->mass(),"gamma_mass","gamma mass", 100,-2,2,  1, "");
        gen_gamma = TLorentzVector(it->px(), it->py(), it->pz(), it->energy());
      }



  }


  if (gen_l1.Pt() > gen_l2.Pt()){
    gen_lPt1 = gen_l1;
    gen_lPt2 = gen_l2;
  }
  else{
    gen_lPt1 = gen_l2;
    gen_lPt2 = gen_l1;
  }

  nel = el_minus.size()+el_plus.size();

  hists->fill1DHist(nel,"gen_nel","Number of gen electrons from a Z", 10,0,10,  1, "");
  TLorentzVector z(0,0,0,0);
  TLorentzVector h(0,0,0,0);
  TLorentzVector l1,l2;


  edm::Handle<reco::ConversionCollection> hConversions;
  iEvent.getByLabel("allConversions", hConversions);

  if (nel>=2 && gen_gamma.Pt()!=0) {

    Handle<reco::GsfElectronCollection > electrons;
    iEvent.getByLabel("gsfElectrons", electrons);

    //Handle<reco::GsfElectronCollection > calibratedElectrons;
    //iEvent.getByLabel(edm::InputTag("calibratedElectrons","calibratedGsfElectrons"), calibratedElectrons);

    //edm::Handle<edm::ValueMap<float>> mvaTrigV0_handle;
    //iEvent.getByLabel("mvaTrigV0", mvaTrigV0_handle);
    //const edm::ValueMap<float> ele_mvaTrigV0 = (*mvaTrigV0_handle.product());

    //edm::Handle<edm::ValueMap<double>> regEne_handle;
    //iEvent.getByLabel(edm::InputTag("eleRegressionEnergy","eneRegForGsfEle"), regEne_handle);
    //const edm::ValueMap<double> ele_regEne = (*regEne_handle.product());

    //edm::Handle<edm::ValueMap<double>> regErr_handle;
    //iEvent.getByLabel(edm::InputTag("eleRegressionEnergy","eneErrorRegForGsfEle"), regErr_handle);
    //const edm::ValueMap<double> ele_regErr = (*regErr_handle.product());


    Handle<reco::TrackCollection> generalTracks;
    iEvent.getByLabel("generalTracks", generalTracks);
    
    Handle<reco::GsfTrackCollection> gsfTracks;
    iEvent.getByLabel("electronGsfTracks", gsfTracks);
    
    Int_t eee=0;
    for (vector<reco::GsfElectron>::const_iterator iElectron = electrons->begin(); iElectron != electrons->end(); ++iElectron) {
      eee++;
      if (iElectron->pt() < 7) continue;
      hists->fill1DHist(iElectron->pt(),"el_pt","pt of an ele", 50,0,200,  1, "");
      TLorentzVector el(iElectron->px(), iElectron->py(), iElectron->pz(), iElectron->energy());

      hists->fill1DHist(el.DeltaR(gen_lPt1),   Form("reco_l%i_gen_lPt1_deltaR",   eee), "reco_gen_l1_deltaR",   100,0,5, 1,"");
      hists->fill1DHist(el.DeltaR(gen_lPt2),   Form("reco_l%i_gen_lPt2_deltaR",   eee), "reco_gen_l2_deltaR",   100,0,5, 1,"");
      hists->fill1DHist(el.DeltaR(gen_gamma),Form("reco_l%i_gen_gamma_deltaR",eee), "reco_gen_gamma_deltaR",100,0,5, 1,"");

      reco::GsfTrackRef gsf = iElectron->gsfTrack();
      //TVector3 t1(gsf->px(), gsf->py(), gsf->pz());

      hists->fill1DHist(fabs(gsf->pt() - gen_lPt1.Pt())/gen_lPt1.Pt() , Form("gsf_l%i_track_ptdiff_1", eee), "pt diff of gsf track and gen lPt1", 100, 0,0.5, 1, "");
      hists->fill1DHist(fabs(gsf->pt() - gen_lPt2.Pt())/gen_lPt2.Pt() , Form("gsf_l%i_track_ptdiff_2", eee), "pt diff of gsf track and gen lPt2", 100, 0,0.5, 1, "");

      Int_t nnn=0;
      if (el.DeltaR(gen_l1) < 0.2 || el.DeltaR(gen_l2) < 0.2)
        for (vector<reco::GsfTrack>::const_iterator gtr = gsfTracks->begin(); gtr != gsfTracks->end(); ++gtr) {
          if (gtr->pt()<5) continue;
          
          TVector3 t(gtr->px(), gtr->py(), gtr->pz());

          hists->fill1DHist(t.DeltaR(el.Vect()), "gsftrack_ele_dR", "track ele dR", 100, 0, 0.5, 1, "");
          if (t.DeltaR(el.Vect())< 0.5)
            nnn++;
        }
      hists->fill1DHist(nnn, "ntracks_in05","n tracks in 0.5 around electron", 10,0,10,1,"");
    }

    hists->fill1DHist(electrons->size(),"reco_nel","Number of reco gsf electrons", 10,0,10,  1, "");


    Handle<vector<reco::Photon> > photons;
    iEvent.getByLabel("photons", photons);
    Int_t ppp = 0;
    for (vector<reco::Photon>::const_iterator iPhoton = photons->begin(); iPhoton != photons->end() ; ++iPhoton) {
      TLorentzVector ph(iPhoton->px(), iPhoton->py(), iPhoton->pz(), iPhoton->p());

      hists->fill1DHist(ph.DeltaR(gen_lPt1),   Form("reco_ph%i_gen_lPt1_deltaR",   ppp),"reco_gen_l1_deltaR",   100,0,5, 1,"");
      hists->fill1DHist(ph.DeltaR(gen_lPt2),   Form("reco_ph%i_gen_lPt2_deltaR",   ppp),"reco_gen_l2_deltaR",   100,0,5, 1,"");
      hists->fill1DHist(ph.DeltaR(gen_gamma),Form("reco_ph%i_gen_gamma_deltaR",ppp),"reco_gen_gamma_deltaR",100,0,5, 1,"");

    }

  }

}


void  EleAnalyzer::beginJob()
{
  outFile = new TFile("output.root","recreate");
  outFile->cd();

  /*
    ktree = new TTree("K","Kevins tree");

  p_lminus = new TLorentzVector();
  p_lplus  = new TLorentzVector();
  p_gamma  = new TLorentzVector();
  ktree->Branch("l_minus",&p_lminus,6400,0);
  ktree->Branch("l_plus",&p_lplus,6400,0);
  ktree->Branch("gamma",&p_gamma,6400,0);

  */
  hists   = new HistManager(outFile);


  outFile->cd();

}

// ------------ method called once each job just after ending the event loop  ------------
void EleAnalyzer::endJob()
{
  outFile->cd();
  hists->writeHists(outFile);
  //ktree->Write();
  outFile->Write();
  outFile->Close();
}

void EleAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&)
{}

void EleAnalyzer::endRun(edm::Run const&, edm::EventSetup const&)
{}

void  EleAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{}

void EleAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{}

void EleAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(EleAnalyzer);
