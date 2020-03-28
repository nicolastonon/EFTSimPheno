/*
## NOTES ##
- Considering at most 1 top + 1 antitop per event
-
*/

// -*- C++ -*-
//
// Package:    myAnalyzer/GenAnalyzer
// Class:      GenAnalyzer
//

/**
 Description: basic gen-level analyzer for pheno studies
*/
//
// Original Author:  Nicolas Tonon
//         Created:  Fri, 06 Dec 2019 13:48:20 GMT
//
//

/* BASH COLORS */
#define RST   "[0m"
#define KRED  "[31m"
#define KGRN  "[32m"
#define KYEL  "[33m"
#define KBLU  "[34m"
#define KMAG  "[35m"
#define KCYN  "[36m"
#define KWHT  "[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "[1m" x RST
#define UNDL(x) "[4m" x RST

// system include files
#include <memory>
#include <vector>
#include <iostream>

#include <Math/Vector3D.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <TVector3.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "SimDataFormats/GeneratorProducts/interface/GenLumiInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenFilterInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h" //https://github.com/cms-sw/cmssw/blob/master/DataFormats/HepMCCandidate/interface/GenParticle.h
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHECommonBlocks.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "GeneratorInterface/LHEInterface/interface/LHERunInfo.h"

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

using namespace std;
using namespace reco;
using namespace edm;

class GenAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
    public:
        explicit GenAnalyzer(const edm::ParameterSet&);
        ~GenAnalyzer();

        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

        const reco::GenParticle* getMother(const reco::GenParticle part);
        int getMotherIndex(Handle<reco::GenParticleCollection>, const reco::GenParticle);
        int getMotherId(Handle<reco::GenParticleCollection>, const reco::GenParticle);
        std::vector<int> GetVectorDaughterIndices(Handle<reco::GenParticleCollection>, const reco::GenParticle);

        float GetDeltaR(float, float, float, float);
        bool isZDecayProduct(Handle<reco::GenParticleCollection>, const reco::GenParticle, int&);
        bool isNonResonant_ll(Handle<reco::GenParticleCollection>, const reco::GenParticle);

        int isTopDecayProduct(Handle<reco::GenParticleCollection>, const reco::GenParticle, int&, int&);
        bool isEleMu(int);
        const GenParticle& GetGenParticle(Handle<reco::GenParticleCollection>, int);
        TLorentzVector GetPtEtaPhiE_fromPartIndex(Handle<reco::GenParticleCollection>, int);
        double Compute_cosThetaStarPol_Top(TLorentzVector, TLorentzVector, TLorentzVector);
        double Compute_cosThetaStarPol_Z(double, double, double, double, double, double, double) ;

    private:
        virtual void beginJob() override;
        virtual void analyze(edm::Event const&, edm::EventSetup const&) override;
        virtual void endJob() override;

        //If you wish to access run-specific information about event generation, you can do so via GenRunInfoProduct
        virtual void beginRun(edm::Run const&, edm::EventSetup const&);
        virtual void endRun(edm::Run const&, edm::EventSetup const&);

        virtual void beginLuminosityBlock(LuminosityBlock const&, EventSetup const&);

        // ----------member data ---------------------------
        // inputs
        edm::EDGetTokenT<reco::GenParticleCollection> genParticleCollectionToken_;
        edm::EDGetTokenT<reco::GenJetCollection> genJetCollection_token_;
        edm::EDGetTokenT<GenEventInfoProduct> genEventInfoProductToken_; //General characteristics of a generated event
        edm::EDGetTokenT<LHEEventProduct> srcToken_; //General characteristics of a generated event (only present if the event starts from LHE events)
        edm::EDGetTokenT<LHERunInfoProduct> LHERunInfoProductToken_;

        string processName;

        float min_pt_jet;
        float min_pt_lep;
        float max_eta_jet;
        float max_eta_lep;

        //Weights
        float mc_weight;
        float mc_weight_originalValue;
        float originalXWGTUP;

//--- OUTPUTS ---
        TFile* ofile_;
        TTree* tree_;

        //All gen particles
        std::vector<float> genParticlesPt_;
        std::vector<float> genParticlesEta_;
        std::vector<float> genParticlesPhi_;
        std::vector<float> genParticlesMass_;
        std::vector<float> genJetsPt_;
        std::vector<float> genJetsEta_;
        std::vector<float> genJetsPhi_;
        std::vector<float> genJetsMass_;

        //Z variables
        int index_Z;
        float Z_pt_;
        float Z_eta_;
        float Z_phi_;
        float Z_m_;
        float Zreco_pt_;
        float Zreco_eta_;
        float Zreco_phi_;
        float Zreco_m_;
        float Zreco_dPhill_;

        //Top variables
        int index_top;
        float Top_pt_;
        float Top_eta_;
        float Top_phi_;
        float Top_m_;

        //AntiTop variables
        int index_antitop;
        float AntiTop_pt_;
        float AntiTop_eta_;
        float AntiTop_phi_;
        float AntiTop_m_;

        //Leading top variables
        float LeadingTop_pt_;
        float LeadingTop_eta_;
        float LeadingTop_phi_;
        float LeadingTop_m_;

        //Full system variables (tZ for tZq process, ttZ for ttZ process)
        float TopZsystem_pt_;
        float TopZsystem_eta_;
        float TopZsystem_phi_;
        float TopZsystem_m_;

        //Recoil jet
        float recoilJet_pt_;
        float recoilJet_eta_;
        float recoilJet_phi_;

        //Polarization variables
        float cosThetaStarPol_Top_;
        float cosThetaStarPol_Z_;

        //Reweights
        std::vector<std::string> v_weightIds_;
        std::vector<float> v_weights_;

        //Others
        int index_Higgs;

        //Also store the total sums of weights (SWE), in histogram
        TH1F* h_SWE;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//--------------------------------------------
//  ######   #######  ##    ##  ######  ######## ########  ##     ##  ######  ########  #######  ########
// ##    ## ##     ## ###   ## ##    ##    ##    ##     ## ##     ## ##    ##    ##    ##     ## ##     ##
// ##       ##     ## ####  ## ##          ##    ##     ## ##     ## ##          ##    ##     ## ##     ##
// ##       ##     ## ## ## ##  ######     ##    ########  ##     ## ##          ##    ##     ## ########
// ##       ##     ## ##  ####       ##    ##    ##   ##   ##     ## ##          ##    ##     ## ##   ##
// ##    ## ##     ## ##   ### ##    ##    ##    ##    ##  ##     ## ##    ##    ##    ##     ## ##    ##
//  ######   #######  ##    ##  ######     ##    ##     ##  #######   ######     ##     #######  ##     ##
//--------------------------------------------

//
// constructors and destructor
//
GenAnalyzer::GenAnalyzer(const edm::ParameterSet& iConfig) :
    LHERunInfoProductToken_(consumes<LHERunInfoProduct,edm::InRun>({"externalLHEProducer"}))
{
    processName = iConfig.getParameter<std::string>("myProcessName");

    cout<<endl<<FYEL("== PROCESS : "<<processName<<" ==")<<endl<<endl;

    //now do what ever initialization is needed
    TString outputname = "output_"+processName+".root";
    ofile_ = new TFile(outputname, "RECREATE","GenAnalyzer output file");
    tree_ = new TTree("tree", "GenAnalyzer output tree");
    h_SWE = new TH1F("h_SWE", "h_SWE", 50, 0, 50); //NB : arbitrary indexing, depends on nof reweights considered !

    tree_->Branch("pt"   , &genParticlesPt_   ) ;
    tree_->Branch("eta"  , &genParticlesEta_  ) ;
    tree_->Branch("phi"  , &genParticlesPhi_  ) ;
    tree_->Branch("mass" , &genParticlesMass_ ) ;

    tree_->Branch("genJetsPt" , &genJetsPt_) ;
    tree_->Branch("genJetsEta" , &genJetsEta_) ;
    tree_->Branch("genJetsPhi" , &genJetsPhi_) ;
    tree_->Branch("genJetsMass" , &genJetsMass_) ;

    tree_->Branch("index_Z" , &index_Z) ;
    tree_->Branch("Z_pt" , &Z_pt_) ;
    tree_->Branch("Z_eta" , &Z_eta_) ;
    tree_->Branch("Z_phi" , &Z_phi_) ;
    tree_->Branch("Z_m" , &Z_m_) ;
    tree_->Branch("Zreco_pt" , &Zreco_pt_) ;
    tree_->Branch("Zreco_eta" , &Zreco_eta_) ;
    tree_->Branch("Zreco_phi" , &Zreco_phi_) ;
    tree_->Branch("Zreco_m" , &Zreco_m_) ;
    tree_->Branch("Zreco_dPhill" , &Zreco_dPhill_) ;

    tree_->Branch("index_top" , &index_top) ;
    tree_->Branch("Top_pt" , &Top_pt_) ;
    tree_->Branch("Top_eta" , &Top_eta_) ;
    tree_->Branch("Top_phi" , &Top_phi_) ;
    tree_->Branch("Top_m" , &Top_m_) ;

    tree_->Branch("index_antitop" , &index_antitop) ;
    tree_->Branch("AntiTop_pt" , &AntiTop_pt_) ;
    tree_->Branch("AntiTop_eta" , &AntiTop_eta_) ;
    tree_->Branch("AntiTop_phi" , &AntiTop_phi_) ;
    tree_->Branch("AntiTop_m" , &AntiTop_m_) ;

    tree_->Branch("LeadingTop_pt" , &LeadingTop_pt_) ;
    tree_->Branch("LeadingTop_eta" , &LeadingTop_eta_) ;
    tree_->Branch("LeadingTop_phi" , &LeadingTop_phi_) ;
    tree_->Branch("LeadingTop_m" , &LeadingTop_m_) ;

    tree_->Branch("TopZsystem_pt" , &TopZsystem_pt_) ;
    tree_->Branch("TopZsystem_eta" , &TopZsystem_eta_) ;
    tree_->Branch("TopZsystem_phi" , &TopZsystem_phi_) ;
    tree_->Branch("TopZsystem_m" , &TopZsystem_m_) ;

    tree_->Branch("recoilJet_pt" , &recoilJet_pt_) ;
    tree_->Branch("recoilJet_eta" , &recoilJet_eta_) ;
    tree_->Branch("recoilJet_phi" , &recoilJet_phi_) ;

    tree_->Branch("cosThetaStarPol_Top" , &cosThetaStarPol_Top_) ;
    tree_->Branch("cosThetaStarPol_Z" , &cosThetaStarPol_Z_) ;

    tree_->Branch("v_weights" , &v_weights_) ;
    tree_->Branch("v_weightIds" , &v_weightIds_) ;

    tree_->Branch("mc_weight" , &mc_weight);
    tree_->Branch("mc_weight_originalValue" , &mc_weight_originalValue);
    tree_->Branch("originalXWGTUP" , &originalXWGTUP);

    tree_->Branch("index_Higgs" , &index_Higgs) ;

    // min_pt_jet  = iConfig.getParameter<double> ("min_pt_jet");
    // min_pt_lep  = iConfig.getParameter<double> ("min_pt_lep");
    // max_eta_jet = iConfig.getParameter<double> ("max_eta_jet");
    // max_eta_lep = iConfig.getParameter<double> ("max_eta_lep");

    //NB : this implementation allows changing inputs without recompiling the code...
    genParticleCollectionToken_ = consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticlesInput"));
    genJetCollection_token_ = consumes<reco::GenJetCollection>(iConfig.getParameter<edm::InputTag>("genJetsInput"));
    genEventInfoProductToken_ = consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("genEventInfoInput"));
    srcToken_ = consumes<LHEEventProduct>(iConfig.getParameter<edm::InputTag>("srcInput"));
}


GenAnalyzer::~GenAnalyzer()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

    h_SWE->Write();
    ofile_->Write();
    delete h_SWE;
    delete ofile_;
}

//--------------------------------------------
//    ###    ##    ##    ###    ##       ##    ## ######## ######## ########
//   ## ##   ###   ##   ## ##   ##        ##  ##       ##  ##       ##     ##
//  ##   ##  ####  ##  ##   ##  ##         ####       ##   ##       ##     ##
// ##     ## ## ## ## ##     ## ##          ##       ##    ######   ########
// ######### ##  #### ######### ##          ##      ##     ##       ##   ##
// ##     ## ##   ### ##     ## ##          ##     ##      ##       ##    ##
// ##     ## ##    ## ##     ## ########    ##    ######## ######## ##     ##
//--------------------------------------------

// ------------ method called for each event  ------------
void GenAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    bool debug = false;

 //  ####  ###### ##### #    # #####
 // #      #        #   #    # #    #
 //  ####  #####    #   #    # #    #
 //      # #        #   #    # #####
 // #    # #        #   #    # #
 //  ####  ######   #    ####  #

    // Event info
    int runNumber_ = iEvent.id().run();
    int lumiBlock_ = iEvent.id().luminosityBlock();
    int eventNumber_ = iEvent.id().event();

    if(debug && eventNumber_ > 10) {return;} //Only debug first events
    if(debug) {cout<<endl<<endl<<endl<<"====== EVENT "<<eventNumber_<<" ======"<<endl;}

    // Initial-state info
    Handle<GenEventInfoProduct> genEventInfoProductHandle;
    iEvent.getByToken(genEventInfoProductToken_, genEventInfoProductHandle);
    // cout<<"genEventInfoProductHandle.isValid() = "<<genEventInfoProductHandle.isValid()<<endl;

    // LHE
    Handle<LHEEventProduct> lheEventProductHandle;
    iEvent.getByToken(srcToken_, lheEventProductHandle);
    // cout<<"lheEventProductHandle.isValid() = "<<lheEventProductHandle.isValid()<<endl;

    Handle<reco::GenParticleCollection> genParticlesHandle; //Smart pointer
    iEvent.getByToken(genParticleCollectionToken_, genParticlesHandle);
    // cout<<"genParticlesHandle.isValid() = "<<genParticlesHandle.isValid()<<endl;

    Handle<reco::GenJetCollection> genJetsHandle; //Smart pointer
    iEvent.getByToken(genJetCollection_token_, genJetsHandle);
    // cout<<"genJetsHandle.isValid() = "<<genJetsHandle.isValid()<<endl;

    genParticlesPt_.clear();
    genParticlesEta_.clear();
    genParticlesPhi_.clear();
    genParticlesMass_.clear();

    genJetsPt_.clear();
    genJetsEta_.clear();
    genJetsPhi_.clear();
    genJetsMass_.clear();

    index_Z = -1;
    Z_pt_ = 0;
    Z_eta_ = 0;
    Z_phi_ = 0;
    Z_m_ = 0;
    Zreco_pt_ = 0;
    Zreco_eta_ = 0;
    Zreco_phi_ = 0;
    Zreco_m_ = 0;
    Zreco_dPhill_ = 0;

    index_top = -1;
    Top_pt_ = 0;
    Top_eta_ = 0;
    Top_phi_ = 0;
    Top_m_ = 0;

    index_antitop = -1;
    AntiTop_pt_ = 0;
    AntiTop_eta_ = 0;
    AntiTop_phi_ = 0;
    AntiTop_m_ = 0;

    LeadingTop_pt_ = 0;
    LeadingTop_eta_ = 0;
    LeadingTop_phi_ = 0;
    LeadingTop_m_ = 0;

    TopZsystem_pt_ = 0;
    TopZsystem_eta_ = 0;
    TopZsystem_phi_ = 0;
    TopZsystem_m_ = 0;

    recoilJet_pt_ = 0;
    recoilJet_eta_ = 0;
    recoilJet_phi_ = 0;

    cosThetaStarPol_Top_ = 0;
    cosThetaStarPol_Z_ = 0;

    v_weights_.clear();
    v_weightIds_.clear();

    mc_weight = 0;
    mc_weight_originalValue = 0;
    originalXWGTUP = 0;

    index_Higgs = -1;
//--------------------------------------------


 // #    # ###### #  ####  #    # #####  ####
 // #    # #      # #    # #    #   #   #
 // #    # #####  # #      ######   #    ####
 // # ## # #      # #  ### #    #   #        #
 // ##  ## #      # #    # #    #   #   #    #
 // #    # ###### #  ####  #    #   #    ####

    mc_weight = 1.;
    if(genEventInfoProductHandle.isValid()) //General characteristics of a generated event
    {
    	// for some 94x amcatnlo samples, the value of the mc weight is not +-1, but some large (pos/neg) number
    	// so we save both the +-1 label and the original number
    	mc_weight_originalValue = genEventInfoProductHandle->weight();
        // cout<<"mc_weight_originalValue = "<<mc_weight_originalValue<<endl;

        mc_weight = (mc_weight_originalValue > 0) ? 1. : -1.;
        // cout<<"mc_weight = "<<mc_weight<<endl;
    }

    if(lheEventProductHandle.isValid()) //General characteristics of a generated event (only present if the event starts from LHE events)
    {
        originalXWGTUP = lheEventProductHandle->originalXWGTUP(); //central event weight
        // cout<<"originalXWGTUP = "<<originalXWGTUP<<endl;

        //NB : the cases "0.5/2" & "2/0.5" are unphysical anti-correlated variations, not needed
        //Now : store scale variations in the order they appear. Then must check manually the order of LHE weights
        /*
        if(lheEventProductHandle->weights().size() > 0)
        {
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[1].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 2 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[2].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 3 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[3].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 4 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[4].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 5 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[5].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 6 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[6].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 7 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[7].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 8 of LHE vector
            cout<<genEventInfoProductHandle->weight()*(lheEventProductHandle->weights()[8].wgt)/(lheEventProductHandle->originalXWGTUP())<<endl; //element 9 of LHE vector
        }
        */

        int nPdfAll = lheEventProductHandle->weights().size();

        float binX = 0.5; //Fill weight in correct histo bin
        for(int w=0; w<nPdfAll; w++)
        {
            const LHEEventProduct::WGT& wgt = lheEventProductHandle->weights().at(w);
            // wgt.wgt * mc_weight_originalValue / originalXWGTUP;

            // cout<<"-- wgt = "<<wgt.wgt<<endl;
            // cout<<"id = "<<wgt.id<<endl;

            TString ts_id = wgt.id;
            // if(ts_id.Contains("ctz", TString::kIgnoreCase) || ts_id.Contains("ctw", TString::kIgnoreCase) || ts_id.Contains("sm", TString::kIgnoreCase) )
            if(ts_id.Contains("rwgt_", TString::kIgnoreCase) )
            {
                // cout<<endl<<"-- wgt = "<<wgt.wgt<<endl;
                // cout<<"id = "<<wgt.id<<endl;

                v_weightIds_.push_back(wgt.id);
                v_weights_.push_back(wgt.wgt);

                h_SWE->Fill(binX, wgt.wgt);
                binX+= 1.;
            }
        }
    } //end lheEventProductHandle.isValid


 //  ####  ###### #    #    #       ####   ####  #####
 // #    # #      ##   #    #      #    # #    # #    #
 // #      #####  # #  #    #      #    # #    # #    #
 // #  ### #      #  # #    #      #    # #    # #####
 // #    # #      #   ##    #      #    # #    # #
 //  ####  ###### #    #    ######  ####   ####  #

//-- Loop on genParticles
    if(genParticlesHandle.isValid() )
    {
        //Gen-level particles
        TLorentzVector Zboson, top, antitop, TopZsystem, leadingTop, recoilJet;

        //Reco from decay products
        TLorentzVector lepZ1, lepZ2; int lepZ1_charge, lepZ2_charge;
        TLorentzVector RecoZ;
        TLorentzVector lepTop, neuTop, bTop;
        TLorentzVector RecoTop;
        TLorentzVector lepAntiTop, neuAntiTop, bAntiTop;
        TLorentzVector RecoAntiTop;
        TLorentzVector RecoTopZsystem;

        // cout<<"genParticlesHandle->size() = "<<genParticlesHandle->size()<<endl;
        // for (auto it = genParticles->begin(); it != genParticles->end(); it++) {
        for(size_t i = 0; i < genParticlesHandle->size(); ++ i)
        {
            const GenParticle & p = (*genParticlesHandle)[i];
            // auto p = *it;

            float ptGen = p.pt();
            float etaGen = p.eta();
            float phiGen = p.phi();
            float mGen = p.mass();
            float EGen = p.energy();
            int idGen = p.pdgId();
            int statusGen = p.status();
            int chargeGen = p.charge();
            int isPromptFinalStateGen = p.isPromptFinalState();
            int isDirectPromptTauDecayProductFinalState = p.isDirectPromptTauDecayProductFinalState();

            //-- Basic cuts
            // if(statusGen != 1) {continue;} //doesn't seem like this is shared by all FS particles (bquark, ...)
            if(ptGen < 2.) {continue;}

            //-- Infos on *all* gen particles
            genParticlesPt_.push_back(ptGen);
            genParticlesEta_.push_back(etaGen);
            genParticlesPhi_.push_back(phiGen);
            genParticlesMass_.push_back(mGen);

            //Access particle's mother infos
            const reco::GenParticle* mom = GenAnalyzer::getMother(p); //Get particle's mother genParticle
            int mother_index = getMotherIndex(genParticlesHandle, p); //Get particle's mother index
            // int mother_id = getMotherId(genParticlesHandle, p); //Get particle's mother ID

            //Get vector of daughters' indices
            std::vector<int> daughter_index = GetVectorDaughterIndices(genParticlesHandle, p);

            //-- Specific printouts
            /*
            if(abs(idGen) == 23) //Z boson : check daughters
            {
                for(size_t idaughter=0; idaughter<daughter_index.size(); idaughter++)
                {
                    cout<<"--> daughter id "<<(*genParticlesHandle)[daughter_index[idaughter]].pdgId()<<endl;
                }
            }
            else if((abs(idGen) == 11 || abs(idGen) == 13) && isPromptFinalStateGen && mother_index != -1) //ele : check mother
            {
                cout<<"---> mother id "<<(*genParticlesHandle)[mother_index].pdgId()<<endl;
            }
            else if(abs(idGen) == 6) //top quarks
            {
                for(size_t idaughter=0; idaughter<daughter_index.size(); idaughter++)
                {
                    cout<<"--> daughter id "<<(*genParticlesHandle)[daughter_index[idaughter]].pdgId()<<endl;
                }
            }
            else if(abs(idGen) == 24) //W bosons
            {
                for(size_t idaughter=0; idaughter<daughter_index.size(); idaughter++)
                {
                    cout<<"--> daughter id "<<(*genParticlesHandle)[daughter_index[idaughter]].pdgId()<<endl;
                }
            }
            */

            if(debug)
            {
                if(abs(idGen) != 11 && abs(idGen) != 13 && abs(idGen) != 15 && abs(idGen) != 23 && abs(idGen) != 24 && abs(idGen) != 6 && abs(idGen) != 5) {continue;} //FIXME

                cout<<endl<<"* ID "<<idGen<<endl;
                cout<<"* mother ID "<<getMotherId(genParticlesHandle, p)<<endl;
                cout<<"* statusGen "<<statusGen<<endl;
                cout<<"* isPromptFinalStateGen "<<isPromptFinalStateGen<<endl;
                cout<<"* isDirectPromptTauDecayProductFinalState "<<isDirectPromptTauDecayProductFinalState<<endl;

                for(unsigned int idaughter=0; idaughter<daughter_index.size(); idaughter++)
                {
                    const GenParticle & daughter = (*genParticlesHandle)[daughter_index[idaughter]];

                    // cout<<"...Daughter index = "<<daughter_index[idaughter]<<endl;
                    cout<<"...Daughter ID = "<<daughter.pdgId()<<endl;
                }
            }
            isPromptFinalStateGen+= isDirectPromptTauDecayProductFinalState; //only check value of 'isPromptFinalStateGen', but care about both cases

 // #####  ######  ####   ####
 // #    # #      #    # #    #
 // #    # #####  #      #    #
 // #####  #      #      #    #
 // #   #  #      #    # #    #
 // #    # ######  ####   ####

            if(abs(idGen) == 25) {index_Higgs = i;} //Look for presence of Higgs bosons in sample

           //-- Particle reco
           // if(isPromptFinalStateGen) //not true for b from top
           {
               //Z BOSON decay products ; consider e,mu ; also consider non-resonant prod
               if(isEleMu(idGen) && isPromptFinalStateGen)
               {
                   if(isZDecayProduct(genParticlesHandle, p, index_Z) || isNonResonant_ll(genParticlesHandle, p)) //Z daughter or ll pair found
                   {
                       if(debug)  cout<<"Found Z decay product (ID="<<idGen<<") / Z index = "<<index_Z<<endl;

                       if(lepZ1.Pt() == 0) //SHOULD IDENTIFY LEPZ1/LEPZ2 AS LEPTON PAIR CLOSEST TO MZ, to include non-resonnant prod ??
                       {
                           // cout<<"lepZ1 pt "<<ptGen<<" eta "<<etaGen<<" phi "<<phiGen<<" E "<<EGen<<endl;
                           lepZ1.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);

                           lepZ1_charge = chargeGen;
                       } //first Z daughter
                       else if(lepZ2.Pt() == 0)
                       {
                           // cout<<"lepZ2 pt "<<ptGen<<" eta "<<etaGen<<" phi "<<phiGen<<" E "<<EGen<<endl;
                           lepZ2.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);

                           lepZ2_charge = chargeGen;
                       } //second Z daughter
                   }
               }

               //TOP QUARK leptonic decay products
               if(isEleMu(idGen) || abs(idGen) == 12 || abs(idGen) == 14 || abs(idGen) == 5) //try to identify the b/l/v from t->bW
               {
                   int isTopAntitopDaughter = isTopDecayProduct(genParticlesHandle, p, index_top, index_antitop);
                   if(isTopAntitopDaughter == 1) //Top daughter found
                   {
                       if(debug) cout<<"Found top (indirect) decay product (ID="<<idGen<<") / Top index = "<<index_top<<endl;

                       if(isEleMu(idGen) && isPromptFinalStateGen) {lepTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //lepton from top decay
                       else if(abs(idGen) == 5) {bTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //bquark from top decay
                       else if(abs(idGen) == 12 || abs(idGen) == 14) {neuTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //neutrino from top decay
                   }
                   else if(isTopAntitopDaughter == -1) //AntiTop daughter found
                   {
                       if(debug) cout<<"Found antitop (indirect) decay product (ID="<<idGen<<") / anti-top index = "<<index_antitop<<endl;

                       if(isEleMu(idGen) && isPromptFinalStateGen) {lepAntiTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //lepton from top decay
                       else if(abs(idGen) == 5) {bAntiTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //bquark from top decay
                       else if(abs(idGen) == 12 || abs(idGen) == 14) {neuAntiTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //neutrino from top decay
                   }
               }

               //RECOIL JET variables (choose leading light jet not from top decay)
               if(abs(idGen) <= 4)
               {
                   int dummy = 0;
                   if(!isTopDecayProduct(genParticlesHandle, p, dummy, dummy) )
                   {
                       if(ptGen > recoilJet.Pt())
                       {
                           recoilJet.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);
                       }
                   }
               }
           }

       } //end GenParticles coll. loop

 // #    # #  ####  #    #       #      ###### #    # ###### #         #    #   ##   #####   ####
 // #    # # #    # #    #       #      #      #    # #      #         #    #  #  #  #    # #
 // ###### # #      ###### ##### #      #####  #    # #####  #         #    # #    # #    #  ####
 // #    # # #  ### #    #       #      #      #    # #      #         #    # ###### #####       #
 // #    # # #    # #    #       #      #       #  #  #      #          #  #  #    # #   #  #    #
 // #    # #  ####  #    #       ###### ######   ##   ###### ######      ##   #    # #    #  ####

        //-- True Z boson variables
        if(index_Z >= 0)
        {
            //True Z boson
            Zboson = GetPtEtaPhiE_fromPartIndex(genParticlesHandle, index_Z);
            Z_pt_ = Zboson.Pt();
            Z_eta_ = Zboson.Eta();
            Z_phi_ = Zboson.Phi();
            Z_m_ = Zboson.M();
        }

        //RECO Z boson variables (also non-resonant ll prod)
        if(lepZ1.Pt() > 0 && lepZ2.Pt() > 0)
        {
            RecoZ = lepZ1+lepZ2;
            Zreco_pt_ = RecoZ.Pt();
            Zreco_eta_ = RecoZ.Eta();
            Zreco_phi_ = RecoZ.Phi();
            Zreco_m_ = RecoZ.M();
            Zreco_dPhill_ = TMath::Abs(lepZ2.Phi() - lepZ1.Phi());

            // if(index_Z < 0) {cout<<"Non-resonant lepton pair found ! Mll = "<<Zreco_m_<<endl;}

            if(lepZ1_charge == -1)
            {
                cosThetaStarPol_Z_ = Compute_cosThetaStarPol_Z(Zreco_pt_, Zreco_eta_, Zreco_phi_, Zreco_m_, lepZ1.Pt(), lepZ1.Eta(), lepZ1.Phi());
            }
            else if(lepZ2_charge == -1)
            {
                cosThetaStarPol_Z_ = Compute_cosThetaStarPol_Z(Zreco_pt_, Zreco_eta_, Zreco_phi_, Zreco_m_, lepZ2.Pt(), lepZ2.Eta(), lepZ2.Phi());
            }
        }

        if(index_top >= 0) //Found top
        {
            top = GetPtEtaPhiE_fromPartIndex(genParticlesHandle, index_top);
            Top_pt_ = top.Pt();
            Top_eta_ = top.Eta();
            Top_phi_ = top.Phi();
            Top_m_ = top.M();

            // RecoTop = lepTop+bTop+neuTop;
            // Top_pt_ = RecoTop.Pt();
            // Top_eta_ = RecoTop.Eta();
            // Top_phi_ = RecoTop.Phi();
            // Top_m_ = RecoTop.M();
        }

        if(index_antitop >= 0) //Found antitop
        {
            antitop = GetPtEtaPhiE_fromPartIndex(genParticlesHandle, index_antitop);
            AntiTop_pt_ = antitop.Pt();
            AntiTop_eta_ = antitop.Eta();
            AntiTop_phi_ = antitop.Phi();
            AntiTop_m_ = antitop.M();

            // RecoAntiTop = lepAntiTop+bAntiTop+neuAntiTop;
            // AntiTop_pt_ = RecoAntiTop.Pt();
            // AntiTop_eta_ = RecoAntiTop.Eta();
            // AntiTop_phi_ = RecoAntiTop.Phi();
            // AntiTop_m_ = RecoAntiTop.M();
        }

        if(index_top >= 0 || index_antitop >= 0)
        {
            //Top-Z system
            if(index_Z >= 0)
            {
                if(index_top >= 0)
                {
                    TopZsystem = Zboson+top;
                    if(index_antitop >= 0) {TopZsystem+= antitop;}
                }
                else if(index_antitop >= 0) {TopZsystem = Zboson+antitop;}

                TopZsystem_pt_ = TopZsystem.Pt();
                TopZsystem_eta_ = TopZsystem.Eta();
                TopZsystem_phi_ = TopZsystem.Phi();
                TopZsystem_m_ = TopZsystem.M();
            }

            //Leading top
            if(top.Pt() > antitop.Pt()) {leadingTop = top; cosThetaStarPol_Top_ = Compute_cosThetaStarPol_Top(leadingTop, lepTop, recoilJet);}
            else {leadingTop = antitop; cosThetaStarPol_Top_ = Compute_cosThetaStarPol_Top(leadingTop, lepAntiTop, recoilJet);}
            LeadingTop_pt_ = leadingTop.Pt();
            LeadingTop_eta_ = leadingTop.Eta();
            LeadingTop_phi_ = leadingTop.Phi();
            LeadingTop_m_ = leadingTop.M();
        } //at least 1 top found

        if(recoilJet.Pt() > 0)
        {
            recoilJet_pt_ = recoilJet.Pt();
            recoilJet_eta_ = recoilJet.Eta();
            recoilJet_phi_ = recoilJet.Phi();
        }

        //NB -- we look for tops and Z bosons via stable final state electrons/muons. So a Z->qq decay will not be found
        if(debug)
        {
            if(index_Z < 0) {cout<<FRED("LEPTONIC (e,u) Z BOSON NOT FOUND !")<<endl;}
            if(index_top < 0) {cout<<FRED("LEPTONIC (e,u) TOP NOT FOUND !")<<endl;}
            if(index_antitop < 0) {cout<<FRED("LEPTONIC (e,u) ANTITOP NOT FOUND !")<<endl;}
        }

       //--Printout daughter infos
       /*
       if(index_Z >= 0)
       {
           std::vector<int> v_Z_daughtersIndices = GetVectorDaughterIndices(genParticlesHandle, GetGenParticle(genParticlesHandle, index_Z) );
           for(unsigned int idaughter=0; idaughter<v_Z_daughtersIndices.size(); idaughter++)
           {
               const GenParticle & daughter = (*genParticlesHandle)[v_Z_daughtersIndices[idaughter]];

               cout<<"Z daughter index = "<<v_Z_daughtersIndices[idaughter]<<endl;
               cout<<"Z daughter ID = "<<daughter.pdgId()<<endl;
           }
       }

       if(index_top >= 0)
       {
           std::vector<int> v_top_daughtersIndices = GetVectorDaughterIndices(genParticlesHandle, GetGenParticle(genParticlesHandle, index_top) );
           for(unsigned int idaughter=0; idaughter<v_top_daughtersIndices.size(); idaughter++)
           {
               const GenParticle & daughter = (*genParticlesHandle)[v_top_daughtersIndices[idaughter]];

               cout<<"Top daughter index = "<<v_top_daughtersIndices[idaughter]<<endl;
               cout<<"Top daughter ID = "<<daughter.pdgId()<<endl;
               cout<<"Top daughter status = "<<daughter.status()<<endl;
               cout<<"Top daughter isPromptFinalState = "<<daughter.isPromptFinalState()<<endl;
           }
       }
       if(index_antitop >= 0)
       {
           std::vector<int> v_antitop_daughtersIndices = GetVectorDaughterIndices(genParticlesHandle, GetGenParticle(genParticlesHandle, index_antitop) );
           for(unsigned int idaughter=0; idaughter<v_antitop_daughtersIndices.size(); idaughter++)
           {
               const GenParticle & daughter = (*genParticlesHandle)[v_antitop_daughtersIndices[idaughter]];

               cout<<"AntiTop daughter index = "<<v_antitop_daughtersIndices[idaughter]<<endl;
               cout<<"AntiTop daughter ID = "<<daughter.pdgId()<<endl;
               cout<<"AntiTop daughter status = "<<daughter.status()<<endl;
               cout<<"AntiTop daughter isPromptFinalState = "<<daughter.isPromptFinalState()<<endl;
           }
       }
       */

    } //end genParticlesHandle.isValid


 //  ####  ###### #    #      # ###### #####  ####
 // #    # #      ##   #      # #        #   #
 // #      #####  # #  #      # #####    #    ####
 // #  ### #      #  # #      # #        #        #
 // #    # #      #   ## #    # #        #   #    #
 //  ####  ###### #    #  ####  ######   #    ####

    /*
    if(genJetsHandle.isValid())
    {
        int nGenJet = genJetsHandle->size();
        for(int ij=0;ij<nGenJet;ij++)
        {
            const reco::GenJet& genJet = genJetsHandle->at(ij);

            genJetsPt_.push_back(genJet.pt());
            genJetsEta_.push_back(genJet.eta());
            genJetsPhi_.push_back(genJet.phi());
            genJetsMass_.push_back(genJet.energy());
        }
    }
    */

    //-- enforce presence of real Z...
    // cout<<"index_Z "<<index_Z<<endl;
    // if(index_Z < 0) {return;}

    tree_->Fill();

    return;
}















//--------------------------------------------
// ########  ########  ######   #### ##    ##
// ##     ## ##       ##    ##   ##  ###   ##
// ##     ## ##       ##         ##  ####  ##
// ########  ######   ##   ####  ##  ## ## ##
// ##     ## ##       ##    ##   ##  ##  ####
// ##     ## ##       ##    ##   ##  ##   ###
// ########  ########  ######   #### ##    ##
//--------------------------------------------

// ------------ method called once each job just before starting event loop  ------------
void GenAnalyzer::beginJob()
{
    cout<<endl<<endl << "[GenAnalyzer::beginJob]" << endl<<endl;
}

// ------------ method called when starting to processes a run  ------------
//NB -- never called ?
void GenAnalyzer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup)
{
    cout<<endl<<endl << "[GenAnalyzer::beginRun]" << endl<<endl;

    //Can printout here infos on all the LHE weights (else comment out)
    //-------------------
    {
        edm::Handle<LHERunInfoProduct> runHandle;
        typedef std::vector<LHERunInfoProduct::Header>::const_iterator headers_const_iterator;

        iRun.getByToken(LHERunInfoProductToken_, runHandle);

        LHERunInfoProduct myLHERunInfoProduct = *(runHandle.product());

        for (headers_const_iterator iter=myLHERunInfoProduct.headers_begin(); iter!=myLHERunInfoProduct.headers_end(); iter++)
        {
            std::cout << iter->tag() << std::endl;
            std::vector<std::string> lines = iter->lines();
            for (unsigned int iLine = 0; iLine<lines.size(); iLine++)
            {
                std::cout << lines.at(iLine);
            }
        }
    }
    //--------------------
}

void GenAnalyzer::beginLuminosityBlock(LuminosityBlock const& iLumi, EventSetup const& iSetup)
{
    cout<<endl<<endl << "[GenAnalyzer::beginLuminosityBlock]" << endl<<endl;
}


//--------------------------------------------
// ######## ##    ## ########
// ##       ###   ## ##     ##
// ##       ####  ## ##     ##
// ######   ## ## ## ##     ##
// ##       ##  #### ##     ##
// ##       ##   ### ##     ##
// ######## ##    ## ########
//--------------------------------------------

// ------------ method called once each job just after ending the event loop  ------------
void GenAnalyzer::endJob()
{
    cout<<endl<<endl << "[GenAnalyzer::endJob]" << endl<<endl;
}


// ------------ method called when ending the processing of a run  ------------
void GenAnalyzer::endRun(edm::Run const& iRun, edm::EventSetup const& iSetup)
{
    cout<<endl<<endl << "[GenAnalyzer::endRun]" << endl<<endl;

    Handle<GenRunInfoProduct> genRunInfo;
    iRun.getByLabel("generator", genRunInfo );
    cout<<"genRunInfo.isValid() "<<genRunInfo.isValid()<<endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void GenAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}












//--------------------------------------------
// ##     ## ######## ##       ########  ######## ########
// ##     ## ##       ##       ##     ## ##       ##     ##
// ##     ## ##       ##       ##     ## ##       ##     ##
// ######### ######   ##       ########  ######   ########
// ##     ## ##       ##       ##        ##       ##   ##
// ##     ## ##       ##       ##        ##       ##    ##
// ##     ## ######## ######## ##        ######## ##     ##
//--------------------------------------------

/**
 * Return the mother genParticle (and make sure it has a different ID)
 */
const reco::GenParticle* GenAnalyzer::getMother(const reco::GenParticle part)
{
    const reco::GenParticle *mom = &part;

    while(mom->numberOfMothers() > 0)
    {
        for(unsigned int j=0; j<mom->numberOfMothers(); ++j)
        {
            mom = dynamic_cast<const reco::GenParticle*>(mom->mother(j));

            if(mom->pdgId() != part.pdgId())
            {
                return mom;
            }
        }
    }

    return mom;
}

//Trick : loop on collection until match mother particle -> Extract mom index
int GenAnalyzer::getMotherIndex(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    int mother_index = 0;

    const reco::GenParticle* mom = getMother(p); //Get particle's mother genParticle

    reco::GenParticleCollection::const_iterator genParticleSrc_m;
    for(genParticleSrc_m = genParticlesHandle->begin(); genParticleSrc_m != genParticlesHandle->end(); genParticleSrc_m++)
    {
        reco::GenParticle *mcp_m = &(const_cast<reco::GenParticle&>(*genParticleSrc_m));

        if(fabs(mcp_m->pt()-mom->pt()) < 10E-6 && fabs(mcp_m->eta()-mom->eta()) < 10E-6) //Matching
        {
            break;
        }

        mother_index++;
    }

    return mother_index;
}

int GenAnalyzer::getMotherId(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    int mother_index = getMotherIndex(genParticlesHandle, p); //Get particle's mother index

    if(mother_index < 0) {return -1;}

    return (*genParticlesHandle)[mother_index].pdgId();
}

bool GenAnalyzer::isZDecayProduct(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p, int& index_Z)
{
    if(abs(getMotherId(genParticlesHandle, p)) == 23) {index_Z = getMotherIndex(genParticlesHandle, p); return true;} //Z daughter found

    return false;
}

//If final-state ele/mu is not from W/Z decay, must be part of the non-resonant pair production
bool GenAnalyzer::isNonResonant_ll(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    if(p.isPromptFinalState()==1 && isEleMu(p.pdgId()) && abs(getMotherId(genParticlesHandle, p)) !=  24 && abs(getMotherId(genParticlesHandle, p)) !=  23)
    {
        // cout<<"NON RESONANT LEPTON FOUND !"<<endl;
        // cout<<"Mother ==== "<<getMotherId(genParticlesHandle, p)<<endl;

        return true;
    }

    return false;
}

int GenAnalyzer::isTopDecayProduct(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p, int& index_top, int& index_antitop)
{
    if(getMotherId(genParticlesHandle, p) == 6) {index_top = getMotherIndex(genParticlesHandle, p); return 1;} //Top daughter found (t -> Wb)
    if(getMotherId(genParticlesHandle, p) == -6) {index_antitop = getMotherIndex(genParticlesHandle, p); return -1;} //AntiTop daughter found (t -> Wb)

    if(abs(getMotherId(genParticlesHandle, p)) == 24) //Look for chain decay daughter (W -> xx)
    {
        const reco::GenParticle* mom = getMother(p); //Get particle's mother genParticle

        if(getMotherId(genParticlesHandle, *mom) == 6) {index_top = getMotherIndex(genParticlesHandle, *mom); return 1;} //Top daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
        if(getMotherId(genParticlesHandle, *mom) == -6) {index_antitop = getMotherIndex(genParticlesHandle, *mom); return -1;} //AntiTop daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
    }

    return 0;
}


//Trick : loop on collection until match daughter particle -> Extract daughter index
//Example here : https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATMCMatchingExercise#RunCode
std::vector<int> GenAnalyzer::GetVectorDaughterIndices(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    std::vector<int> daughter_index;

    const reco::GenParticleRefVector& daughterRefs = p.daughterRefVector();
    for(reco::GenParticleRefVector::const_iterator idr = daughterRefs.begin(); idr!= daughterRefs.end(); ++idr)
    {
        if(idr->isAvailable())
        {
            const reco::GenParticleRef& genParticle = (*idr);
            const reco::GenParticle *d = genParticle.get();

            reco::GenParticleCollection::const_iterator genParticleSrc_s;

            int index = 0;
            for(genParticleSrc_s = genParticlesHandle->begin(); genParticleSrc_s != genParticlesHandle->end(); genParticleSrc_s++)
            {
                reco::GenParticle *mcp_s = &(const_cast<reco::GenParticle&>(*genParticleSrc_s));

                if(fabs(mcp_s->pt()-(*d).pt()) < 10E-6 && fabs(mcp_s->eta()-(*d).eta()) < 10E-6) //Matching
                {
                    break;
                }

                index++;
            }

            daughter_index.push_back(index);
        }
    }

    return daughter_index;
}

float GenAnalyzer::GetDeltaR(float eta1,float phi1,float eta2,float phi2)
{
   float DeltaPhi = TMath::Abs(phi2 - phi1);
   if (DeltaPhi > 3.141593 ) DeltaPhi -= 2.*3.141593;
   return TMath::Sqrt( (eta2-eta1)*(eta2-eta1) + DeltaPhi*DeltaPhi );
}

bool GenAnalyzer::isEleMu(int id)
{
    id = abs(id);

    if(id == 11 || id == 13) {return true;}

    return false;
}

const GenParticle& GenAnalyzer::GetGenParticle(Handle<reco::GenParticleCollection> genParticlesHandle, int index)
{
    if(index < 0) {cout<<"ERROR : particle index < 0 !"<<endl;}
    return (*genParticlesHandle)[index];
}

TLorentzVector GenAnalyzer::GetPtEtaPhiE_fromPartIndex(Handle<reco::GenParticleCollection> genParticlesHandle, int index)
{
    TLorentzVector tmp;

    if(index < 0) {return tmp;}

    const GenParticle & p = (*genParticlesHandle)[index];

    float ptGen = p.pt();
    float etaGen = p.eta();
    float phiGen = p.phi();
    // float mGen = p.mass();
    float EGen = p.energy();

    tmp.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);

    return tmp;
}

//From Potato code : .../tzq/include/Observable.h
// Top quark polarization angle theta*_pol (see TOP-17-023)
double GenAnalyzer::Compute_cosThetaStarPol_Top(TLorentzVector t, TLorentzVector lep, TLorentzVector spec)
{
    // boost into top rest frame
    const TVector3 lvBoost = -t.BoostVector();
    lep.Boost(lvBoost);
    spec.Boost(lvBoost);

    // cout<<"t.Pt() "<<t.Pt()<<endl;
    // cout<<"lep.Pt() "<<lep.Pt()<<endl;
    // cout<<"spec.Pt() "<<spec.Pt()<<endl;
    // cout<<"spec.Vect()*lep.Vect() "<<spec.Vect()*lep.Vect()<<endl;
    // cout<<"spec.Vect().Mag() * lep.Vect().Mag() "<<spec.Vect().Mag() * lep.Vect().Mag()<<endl;

    if(!spec.Vect().Mag() || !lep.Vect().Mag()) {return -99;}

    return (spec.Vect() * lep.Vect()) / (spec.Vect().Mag() * lep.Vect().Mag());
}

//From Potato code : .../ttz3l/include/CosThetaStar.h
//Z boson polarization angle (see TOP-18-009)
double GenAnalyzer::Compute_cosThetaStarPol_Z(double z_pt, double z_eta, double z_phi, double z_mass, double neg_pt, double neg_eta, double neg_phi)
{
    // Compute cosTheta
    const TVector3 z_3d(
        z_pt*TMath::Cos(z_phi),
        z_pt*TMath::Sin(z_phi),
        z_pt*TMath::SinH(z_eta)
    );
    const TVector3 neg_3d(
        neg_pt*TMath::Cos(neg_phi),
        neg_pt*TMath::Sin(neg_phi),
        neg_pt*TMath::SinH(neg_eta)
    );

    const double cosTheta = z_3d*neg_3d / TMath::Sqrt(z_3d*z_3d) / TMath::Sqrt(neg_3d*neg_3d);

    // Compute Lorentz boost
    const double gammasq = 1.0+TMath::Sq(z_pt/z_mass*TMath::CosH(z_eta));
    const double beta = TMath::Sqrt(1.0-1.0/gammasq);

    // Compute cosThetaStar
    return (-beta+cosTheta) / (1-beta*cosTheta);
}


//define this as a plug-in
DEFINE_FWK_MODULE(GenAnalyzer);