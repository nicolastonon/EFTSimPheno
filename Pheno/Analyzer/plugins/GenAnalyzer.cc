/**
## NOTES ##
- Considering at most 1 top + 1 antitop per event
- Not applying any event reweighting in this code (only filling output TTree) --> Distributions in TBrowser correspond to arbitrary baseline EFT point !
- NB: can't access MET at GEN-level (need PF info). Available collections are genMetTrue & genMetCalo (but exclude neutrinos ? cf. https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideDataFormatRecoMET)
**/

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

bool debug = false; //Global debug variable
float DEFVAL = -9; //Default value
int count_events = 0; //Global event counter

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
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHECommonBlocks.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "GeneratorInterface/LHEInterface/interface/LHERunInfo.h"

// #include "TThreadSlots.h"
// #include "TROOT.h"
// #include "Compression.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"
#include <Math/Vector3D.h>
#include <TLorentzVector.h>
#include <TVector3.h>

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

        const reco::GenParticle* getTrueMother(const reco::GenParticle part);
        int getTrueMotherIndex(Handle<reco::GenParticleCollection>, const reco::GenParticle);
        int getTrueMotherId(Handle<reco::GenParticleCollection>, const reco::GenParticle);
        std::vector<int> GetVectorDaughterIndices(Handle<reco::GenParticleCollection>, const reco::GenParticle);

        float GetDeltaR(float, float, float, float);
        float GetDeltaEta(TLorentzVector, TLorentzVector);
        bool isZDecayProduct(Handle<reco::GenParticleCollection>, const reco::GenParticle, int&);
        bool isFinalStateElMu(Handle<reco::GenParticleCollection>, const reco::GenParticle);

        int isTopDecayProduct(Handle<reco::GenParticleCollection>, const reco::GenParticle);
        int isTopOrAntitopDecayProduct(Handle<reco::GenParticleCollection>, const reco::GenParticle, int&, int&);
        bool isEleMu(int);
        const GenParticle& GetGenParticle(Handle<reco::GenParticleCollection>, int);
        TLorentzVector GetTLorentzVector_fromPartIndex(Handle<reco::GenParticleCollection>, int);
        double Compute_cosThetaStarPol_Top(TLorentzVector, TLorentzVector, TLorentzVector);
        double Compute_cosThetaStarPol_Z(TLorentzVector, TLorentzVector);
        void SetBranches(TTree*);
        void Fill_HighLevel_Variables(TLorentzVector Zboson, TLorentzVector RecoZ, TLorentzVector lepTop, TLorentzVector hadTop, TLorentzVector TopZsystem, TLorentzVector recoilQuark, TLorentzVector lepZ1, TLorentzVector lepZ2, TLorentzVector lepTopl, TLorentzVector lepTopnu, TLorentzVector lepTopb, TLorentzVector lepTopW, TLorentzVector hadTopq1, TLorentzVector hadTopq2, TLorentzVector hadTopb, int lepZ1_id, int lepZ2_id, int lepTopl_id);
        bool Event_Selection(int, bool, bool, bool,TLorentzVector, TLorentzVector, int, int);
        void Get_OtherDecayProducts_LepTop(Handle<reco::GenParticleCollection>, int, TLorentzVector&, TLorentzVector&, TLorentzVector&, int&);
        void Get_OtherDecayProducts_HadTop(Handle<reco::GenParticleCollection>, int, TLorentzVector&, TLorentzVector&, TLorentzVector&);
        void Init_Variables();

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

        //--- TFileService (output file)
        edm::Service<TFileService> fs;

        // TFile* ofile_;
        TTree* tree_;

        string processName;

        float min_pt_jet;
        float min_pt_lep;
        float max_eta_jet;
        float max_eta_lep;

        //All gen particles
        // std::vector<float> genParticlesPt_;
        // std::vector<float> genParticlesEta_;
        // std::vector<float> genParticlesPhi_;
        // std::vector<float> genParticlesMass_;

        std::vector<float> genJetsPt_;
        std::vector<float> genJetsEta_;
        std::vector<float> genJetsPhi_;
        std::vector<float> genJetsMass_;

        //Weights
        float mc_weight;
        float mc_weight_originalValue;
        float originalXWGTUP;

        //Reweights
        std::vector<std::string> v_weightIds_;
        std::vector<float> v_weights_;

        //Also store the total sums of weights (SWE), in histogram
        TH1F* h_SWE;

//--------------------------------------------
//VARIABLES

        //Gen Z boson
        int Z_decayMode_;
        float Z_pt_;
        float Z_eta_;
        float Z_phi_;
        float Z_m_;

        //Reco Z boson = (l1+l2)
        float Zreco_pt_;
        float Zreco_eta_;
        float Zreco_phi_;
        float Zreco_m_;
        float Zreco_dPhill_;
        float lepZ1_pt_;
        float lepZ2_pt_;

        //Leptonic top variables
        float LepTop_pt_;
        float LepTop_eta_;
        float LepTop_phi_;
        float LepTop_m_;

        //Lepton decay from top variables
        float LepTopLep_pt_;
        float LepTopLep_eta_;
        float LepTopLep_phi_;

        //Top(lep)+Z system
        float TopZsystem_pt_;
        float TopZsystem_eta_;
        float TopZsystem_phi_;
        float TopZsystem_m_;

        //Recoil jet //NB: useful for tZq only
        float recoilQuark_pt_;
        float recoilQuark_eta_;
        float recoilQuark_phi_;
        float recoilQuark_id;

        //b-quark from top
        float LepTopB_pt_;
        float LepTopB_eta_;
        float LepTopB_phi_;
        
        //Neutrino from top
    	float lepTopnu_pt_;

        //W boson from leptonic top decay
        float mTW_;
        float Wlep_pt_;
        float Wlep_eta_;
        float Wlep_phi_;

        //Polarization variables
        float cosThetaStarPol_Top_;
        float cosThetaStarPol_Z_;

        //Masses
        float Mass_3l_;

        //Min/max dijet system variables
        float maxDiJet_pt_, maxDiJet_m_;
        float minDiJet_pt_, minDiJet_m_;
        float minDiJet_dEta_, maxDiJet_dEta_;
        float minDiJet_dPhi_, maxDiJet_dPhi_;
        float minDiJet_dR_, maxDiJet_dR_;

        //Angles, distances
        float dR_tZ_;
        float dR_ZlW_;
        float dR_blW_;
        float dR_bW_;
        float dR_tClosestLep_;
        float dR_tClosestJet_;
        float dR_jprimeClosestLep_;
        float dEta_jprimeClosestLep_;
        float dR_tjprime_, dEta_tjprime_;
        float dR_bjprime_, dEta_bjprime_;
        float dR_lWjprime_, dEta_lWjprime_;
        float dR_Zjprime_, dEta_Zjprime_;
        float maxEtaJet_;
        float maxDelPhiLL_;
        float maxDelRbL_;

        //Specific to ttZ
        float dR_ttbar_, dEta_ttbar_;

        //Jet multiplicities
        float njets_;

        //Others
        float index_Higgs, index_Z;
        float leptonCharge_;
        float channel_;
        float ptLepSum_;
        float ptHadSum_;
        float mHT_;
        float lepAsym_;
        // float mjj_max_;

        // float METpt_; //What to use : genMetTrue / genMetCalo ?
};


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

    //If want to create output file interactively (does not work with crab ?)
    // TString outputname = "output_"+processName+".root";
    // ofile_ = new TFile(outputname, "RECREATE","GenAnalyzer output file");
    // tree_ = new TTree("tree", "GenAnalyzer output tree");
    // h_SWE = new TH1F("h_SWE", "h_SWE", 50, 0, 50); //NB : arbitrary indexing, depends on nof reweights considered !

// ########################
// #  Create output tree  #
// ########################

    tree_ = fs->make<TTree>("tree","GenAnalyzer output tree");
    h_SWE = fs->make<TH1F>("h_SWE", "h_SWE", 200,  0., 200);

    SetBranches(tree_);

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

    // h_SWE->Write();
    // ofile_->Write();
    // delete h_SWE;
    // delete ofile_;
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
 //  ####  ###### ##### #    # #####
 // #      #        #   #    # #    #
 //  ####  #####    #   #    # #    #
 //      # #        #   #    # #####
 // #    # #        #   #    # #
 //  ####  ######   #    ####  #

    // Event info
    // int runNumber_ = iEvent.id().run();
    // int lumiBlock_ = iEvent.id().luminosityBlock();
    int eventNumber_ = iEvent.id().event();

    // Initial-state info
    Handle<GenEventInfoProduct> genEventInfoProductHandle;
    iEvent.getByToken(genEventInfoProductToken_, genEventInfoProductHandle);
    // cout<<"genEventInfoProductHandle.isValid() = "<<genEventInfoProductHandle.isValid()<<endl;

    // LHE
    Handle<LHEEventProduct> lheEventProductHandle;
    iEvent.getByToken(srcToken_, lheEventProductHandle);

    Handle<reco::GenParticleCollection> genParticlesHandle; //Smart pointer
    iEvent.getByToken(genParticleCollectionToken_, genParticlesHandle);

    Handle<reco::GenJetCollection> genJetsHandle; //Smart pointer
    iEvent.getByToken(genJetCollection_token_, genJetsHandle);
    
    if(debug)
    {
        if(count_events > 50) {return;} //Only debug first few events
        cout<<endl<<endl<<endl<<endl<<endl<<"--------------------------------------------"<<endl;
        cout<<endl<<"====== EVENT "<<eventNumber_<<" ======"<<endl;
        cout<<"genJetsHandle.isValid() = "<<genJetsHandle.isValid()<<endl;
        cout<<"lheEventProductHandle.isValid() "<<lheEventProductHandle.isValid()<<endl;
        cout<<"genJetsHandle.isValid() "<<genJetsHandle.isValid()<<endl;
    	count_events++; //Use this event counter to debug only N events
    }
    if(!genParticlesHandle.isValid()) {cout<<"genParticlesHandle.isValid() == false ! Return !"<<endl; return;}

    Init_Variables();

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

        mc_weight = (mc_weight_originalValue > 0) ? 1. : -1.; //Not used
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
            //cout<<"id = "<<wgt.id<<endl;

            TString ts_id = wgt.id;
            // if(ts_id.Contains("ctz", TString::kIgnoreCase) || ts_id.Contains("ctw", TString::kIgnoreCase) || ts_id.Contains("sm", TString::kIgnoreCase) )
            if(ts_id.Contains("rwgt_", TString::kIgnoreCase) || ts_id.Contains("EFTrwgt", TString::kIgnoreCase) )
            {
                //cout<<endl<<"-- wgt = "<<wgt.wgt<<endl;
                //cout<<"id = "<<wgt.id<<endl;

                v_weightIds_.push_back(wgt.id);
                v_weights_.push_back(wgt.wgt);

                h_SWE->Fill(binX, wgt.wgt);
                binX+= 1.;
            }
        }
    } //end lheEventProductHandle.isValid


//  ######   ######## ##    ##    ##        #######   #######  ########
// ##    ##  ##       ###   ##    ##       ##     ## ##     ## ##     ##
// ##        ##       ####  ##    ##       ##     ## ##     ## ##     ##
// ##   #### ######   ## ## ##    ##       ##     ## ##     ## ########
// ##    ##  ##       ##  ####    ##       ##     ## ##     ## ##
// ##    ##  ##       ##   ###    ##       ##     ## ##     ## ##
//  ######   ######## ##    ##    ########  #######   #######  ##

//-- Loop on genParticles
    //OLD
    // TLorentzVector Zboson, top, antitop, TopZsystem, leadingTop, recoilQuark;
    // TLorentzVector lepZ1, lepZ2; int lepZ1_id=-99, lepZ2_id=-99; //Leptons from Z, or from non-resonant prod.
    // TLorentzVector lTop, neuTop, bTop;
    // TLorentzVector RecoTop; //t = b+v+l
    // TLorentzVector lAntiTop, neuAntiTop, bAntiTop;
    // TLorentzVector RecoAntiTop; //t = b+v+l

    // int index_top=-1, index_antitop=-1;
    int index_Z=-1, index_lepTop=-1, index_hadTop=-1, idx_W=-1;
    int lepZ1_id=-99, lepZ2_id=-99,lepTopl_id=-99;

    //-- TLorentzVectors
    TLorentzVector Zboson, RecoZ, lepTop, hadTop, TopZsystem;
    TLorentzVector recoilQuark;
    TLorentzVector lepZ1, lepZ2;
    TLorentzVector lepTopl, lepTopnu, lepTopb, lepTopW;
    TLorentzVector hadTopq1, hadTopq2, hadTopb;

    int nEleMuFinalState = 0;

    if(debug) cout<<"genParticlesHandle->size() = "<<genParticlesHandle->size()<<endl;
    // for (auto it = genParticles->begin(); it != genParticles->end(); it++) {
    for(size_t i = 0; i < genParticlesHandle->size(); ++ i)
    {
        const GenParticle & p = (*genParticlesHandle)[i];
        // auto p = *it;

        float ptGen = p.pt();
        float etaGen = p.eta();
        float phiGen = p.phi();
        float EGen = p.energy();
        int idGen = p.pdgId();
        int statusGen = p.status();
        int isPromptFinalStateGen = p.isPromptFinalState();
        int isDirectPromptTauDecayProductFinalState = p.isDirectPromptTauDecayProductFinalState();
        // float mGen = p.mass();
        // int chargeGen = p.charge();

        //-- Basic cuts
        // if(statusGen >= 71 && statusGen <= 79) {continue;} //NB : not all FS particles have statusGen==1 ! see : http://home.thep.lu.se/~torbjorn/pythia81html/ParticleProperties.html
        if(ptGen < 5.) {continue;} //In analysis, will not reconstruct leptons below 10 GeV... //NB : this cut may remove prompt leptons, e.g. from Z->tau->X decay

        //-- Infos on *all* gen particles
        // genParticlesPt_.push_back(ptGen);
        // genParticlesEta_.push_back(etaGen);
        // genParticlesPhi_.push_back(phiGen);
        // genParticlesMass_.push_back(mGen);

        //Get vector of daughters' indices
        std::vector<int> daughter_indices = GetVectorDaughterIndices(genParticlesHandle, p);

        if(debug)
        {
            //Access particle's mother infos
            // const reco::GenParticle* mom = GenAnalyzer::getTrueMother(p); //Get particle's mother genParticle
            // int mother_index = getTrueMotherIndex(genParticlesHandle, p); //Get particle's mother index
            // int mother_id = getTrueMotherId(genParticlesHandle, p); //Get particle's mother ID

            // if(
            // abs(idGen) != 11
            // && abs(idGen) != 13
            // && abs(idGen) != 15
            // && abs(idGen) != 23
            // && abs(idGen) != 24
            // && abs(idGen) != 25
            // && abs(idGen) != 6
            // && abs(idGen) != 5
            // ) {continue;}

            if(
                abs(idGen) > 25
                || abs(idGen) == 22
                || abs(idGen) == 21
            ) {continue;}

            if(daughter_indices.size() > 0 && (*genParticlesHandle)[daughter_indices[0]].pdgId() == idGen) {continue;} //don't printout particle if it 'decays into itself'

            cout<<endl<<"* ID "<<idGen<<" (idx "<<i<<")"<<endl;
            cout<<"* Mother ID "<<getTrueMotherId(genParticlesHandle, p)<<endl;
            cout<<"* statusGen "<<statusGen<<" / promptFS "<<isPromptFinalStateGen<<" / promptTauFS "<<isDirectPromptTauDecayProductFinalState<<" / pt "<<ptGen<<endl;

            for(unsigned int idaughter=0; idaughter<daughter_indices.size(); idaughter++)
            {
                const GenParticle & daughter = (*genParticlesHandle)[daughter_indices[idaughter]];

                // cout<<"...Daughter index = "<<daughter_indices[idaughter]<<endl;
                cout<<"... daughter ID = "<<daughter.pdgId()<<" (idx "<<daughter_indices[idaughter]<<")"<<endl;
            }
        }

        isPromptFinalStateGen+= isDirectPromptTauDecayProductFinalState; //will only check value of 'isPromptFinalStateGen', but care about both cases

// #####  ######  ####   ####
// #    # #      #    # #    #
// #    # #####  #      #    #
// #####  #      #      #    #
// #   #  #      #    # #    #
// #    # ######  ####   ####

       //-- Particle reconstruction

       //Look for presence of Higgs bosons in sample
       if(abs(idGen) == 25) {index_Higgs = i;}

       else if(abs(idGen) == 23) //Z boson
       {
           if(daughter_indices.size() > 0) {Z_decayMode_ = (*genParticlesHandle)[daughter_indices[0]].pdgId();}
       }

       //Look for final-state e,mu
       else if(isEleMu(idGen) && isPromptFinalStateGen)
       {
            nEleMuFinalState++;

            //CHECK IF COMES FROM TOP QUARK DECAY
            index_lepTop = isTopDecayProduct(genParticlesHandle, p);
            if(index_lepTop >= 0) //e,mu from top decay found
            {
                if(debug) {cout<<FYEL("Found leptonic top quark --> Index = "<<index_lepTop<<"")<<endl;}
                lepTopl.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen); //Lepton from top decay
                lepTopl_id = idGen;
                continue;
            }

           //CHECK IF COMES FROM Z BOSON DECAY (or non-resonant ll prod.)
           if(isZDecayProduct(genParticlesHandle, p, index_Z) || isFinalStateElMu(genParticlesHandle, p)) //Z daughter or ll pair found
           {
               if(debug) {cout<<FYEL("Found lepton (--> Z index = "<<index_Z<<")")<<endl;}

               if(lepZ1.Pt() == 0) //Check whether lepZ1 TLVec is already filled
               {
                   // cout<<"lepZ1 pt "<<ptGen<<" eta "<<etaGen<<" phi "<<phiGen<<" E "<<EGen<<endl;
                   lepZ1.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);
                   lepZ1_id = idGen;
               }
               else if(lepZ2.Pt() == 0) //Check whether lepZ2 TLVec is already filled
               {
                   // cout<<"lepZ2 pt "<<ptGen<<" eta "<<etaGen<<" phi "<<phiGen<<" E "<<EGen<<endl;
                   lepZ2.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);
                   lepZ2_id = idGen;
               }
           }
       }

       // Look for neutrinos and b quarks from top decay
       /*
       else if(abs(idGen) == 12 || abs(idGen) == 14 || abs(idGen) == 5) //try to identify the b/l/v from t->bW
       {
           int isTopAntitopDaughter = isTopOrAntitopDecayProduct(genParticlesHandle, p, index_top, index_antitop);
           if(isTopAntitopDaughter == 1) //top decay product found
           {
               if(abs(idGen) == 5) {bTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //bquark from top decay
               else if(abs(idGen) == 12 || abs(idGen) == 14) {neuTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //neutrino from top decay
           }
           else if(isTopAntitopDaughter == -1) //antitop decay product found
           {
               if(abs(idGen) == 5) {bAntiTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //bquark from antitop decay
               else if(abs(idGen) == 12 || abs(idGen) == 14) {neuAntiTop.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen);} //neutrino from antitop decay
           }
       }
       */

       //Look for light recoil jet (--> select leading light jet not coming from top decay)
       //Look for hadronic top decay products
       else if(abs(idGen) <= 4) //Consider u,d,c,s flavours only
       {
           int idx_tmp = isTopDecayProduct(genParticlesHandle, p);

           if(idx_tmp == -1)
           {
                int momid = abs(getTrueMotherId(genParticlesHandle, p));
                if(momid!=21 && momid!=24 && momid!=0)
                {
                    //Hardest light quark not from top, W, gg <-> recoil jet
                    if(ptGen > recoilQuark.Pt())
                    {
                        recoilQuark.SetPtEtaPhiE(ptGen, etaGen, phiGen, EGen); recoilQuark_id = idGen; //Select hardest possible jet
                        if(debug) {cout<<FYEL("Found recoil jet, id = "<<idGen<<"")<<endl;}
                    }
                }
           }
           else
           {
               index_hadTop = idx_tmp;
               if(debug) {cout<<FYEL("Found hadronic top --> index = "<<index_hadTop<<"")<<endl;}
           }
       }

   } //end GenParticles coll. loop

//--------------------------------------------

   // Z_decayMode_ = abs(lepZ1_id);

   //true <-> particle decays leptonically; if decays into tau, tau then decays into e,mu
   // bool hasTopDecayEMU = false;
   // bool hasAntitopDecayEMU = false;
   // bool hasZDecayEMU = false;
   // if(lepZ1.Pt() > 0 && lepZ2.Pt() > 0 && lepZ1_id == -lepZ2_id && abs((lepZ1+lepZ2).M()-91.2)<15) {hasZDecayEMU = true;} //SFOS pair within 15 GeV of Z peak
   // if(lTop.Pt() > 0) {hasTopDecayEMU = true;}
   // if(lAntiTop.Pt() > 0) {hasAntitopDecayEMU = true;}

   bool hasLepTop = (index_lepTop != -1);
   bool hasHadTop = (index_hadTop != -1);
   bool hasZDecayEMU = false;
   if(lepZ1.Pt() > 0 && lepZ2.Pt() > 0 && lepZ1_id == -lepZ2_id && abs((lepZ1+lepZ2).M()-91.2)<15) {hasZDecayEMU = true;} //SFOS pair within 15 GeV of Z peak

   if(Event_Selection(nEleMuFinalState, hasLepTop, hasHadTop, hasZDecayEMU, lepZ1, lepZ2, lepZ1_id, lepZ2_id) == false) {return;}

   Get_OtherDecayProducts_LepTop(genParticlesHandle, index_lepTop, lepTopnu, lepTopb, lepTopW, idx_W);
   Get_OtherDecayProducts_HadTop(genParticlesHandle, index_hadTop, hadTopq1, hadTopq2, hadTopb);


// #       ####  #    #       #      ###### #    # ###### #         #    #   ##   #####   ####
// #      #    # #    #       #      #      #    # #      #         #    #  #  #  #    # #
// #      #    # #    # ##### #      #####  #    # #####  #         #    # #    # #    #  ####
// #      #    # # ## #       #      #      #    # #      #         #    # ###### #####       #
// #      #    # ##  ##       #      #       #  #  #      #          #  #  #    # #   #  #    #
// ######  ####  #    #       ###### ######   ##   ###### ######      ##   #    # #    #  ####

    //-- True Z boson variables
    if(index_Z >= 0)
    {
        //True Z boson
        Zboson = GetTLorentzVector_fromPartIndex(genParticlesHandle, index_Z);
        Z_pt_ = Zboson.Pt();
        Z_eta_ = Zboson.Eta();
        Z_phi_ = Zboson.Phi();
        Z_m_ = Zboson.M();
    }

    //Reco (ee,uu) Z boson variables
    if(hasZDecayEMU)
    {
        RecoZ = lepZ1+lepZ2;
        Zreco_pt_ = RecoZ.Pt();
        Zreco_eta_ = RecoZ.Eta();
        Zreco_phi_ = RecoZ.Phi();
        Zreco_m_ = RecoZ.M();
        // Zreco_dPhill_ = TMath::Abs(lepZ2.Phi() - lepZ1.Phi());
        Zreco_dPhill_ = abs(lepZ1.DeltaPhi(lepZ2));
        lepZ1_pt_ = lepZ1.Pt(); lepZ2_pt_ = lepZ2.Pt(); 

        if(debug && index_Z < 0) {cout<<"Non-resonant lepton pair found ! Mll = "<<Zreco_m_<<endl;}
    }
    else {if(debug) {cout<<FRED("lepZ1.Pt() "<<lepZ1.Pt()<<"")<<endl; cout<<FRED("lepZ2.Pt() "<<lepZ2.Pt()<<"")<<endl;}} //Can happen e.g. if 1 tau lepton decays hadronically

    // if(index_top >= 0) //Found top
    // {
    //     top = GetTLorentzVector_fromPartIndex(genParticlesHandle, index_top);
    //     Top_pt_ = top.Pt();
    //     Top_eta_ = top.Eta();
    //     Top_phi_ = top.Phi();
    //     Top_m_ = top.M();
    // }
    // if(index_antitop >= 0) //Found antitop
    // {
    //     antitop = GetTLorentzVector_fromPartIndex(genParticlesHandle, index_antitop);
    //     AntiTop_pt_ = antitop.Pt();
    //     AntiTop_eta_ = antitop.Eta();
    //     AntiTop_phi_ = antitop.Phi();
    //     AntiTop_m_ = antitop.M();
    // }

    //Leptonic top
    if(hasLepTop)
    {
        lepTop = GetTLorentzVector_fromPartIndex(genParticlesHandle, index_lepTop);
        LepTop_pt_ = lepTop.Pt();
        LepTop_eta_ = lepTop.Eta();
        LepTop_phi_ = lepTop.Phi();
        LepTop_m_ = lepTop.M();
    }

    //TopZsystem -- or use recoZ ?
    if(hasLepTop)
    {
        TopZsystem_pt_ = (Zboson+lepTop).Pt();
        TopZsystem_eta_ = (Zboson+lepTop).Eta();
        TopZsystem_phi_ = (Zboson+lepTop).Phi();
        TopZsystem_m_ = (Zboson+lepTop).M();
    }
    else if(hasHadTop)
    {
        hadTop = GetTLorentzVector_fromPartIndex(genParticlesHandle, index_hadTop);
        TopZsystem_pt_ = (Zboson+hadTop).Pt();
        TopZsystem_eta_ = (Zboson+hadTop).Eta();
        TopZsystem_phi_ = (Zboson+hadTop).Phi();
        TopZsystem_m_ = (Zboson+hadTop).M();
    }

    if(recoilQuark.Pt() > 0)
    {
        recoilQuark_pt_ = recoilQuark.Pt();
        recoilQuark_eta_ = recoilQuark.Eta();
        recoilQuark_phi_ = recoilQuark.Phi();
    }

    if(lepTopl.Pt() > 0)
    {
        LepTopLep_pt_ = lepTopl.Pt();
        LepTopLep_eta_ = lepTopl.Eta();
        LepTopLep_phi_ = lepTopl.Phi();
    }

    if(lepTopb.Pt() > 0)
    {
        LepTopB_pt_ = lepTopb.Pt();
        LepTopB_eta_ = lepTopb.Eta();
        LepTopB_phi_ = lepTopb.Phi();
    }

    if(lepTopW.Pt() > 0)
    {
        mTW_ = lepTopW.Mt(); //Transverse mass
        Wlep_pt_ = lepTopW.Pt();
        Wlep_eta_ = lepTopW.Eta();
        Wlep_phi_ = lepTopW.Phi();
    }

    //NB -- we look for tops and Z bosons via stable final state electrons/muons. So a Z->qq decay will not be found
    if(debug)
    {
        cout<<endl<<endl<<FMAG("=== Event summary ===")<<endl;
        if(index_Z > 0) {cout<<"LEPTONIC (e,u) Z BOSON FOUND ! ("<<index_Z<<")"<<endl;}
        if(index_lepTop > 0) {cout<<"LEPTONIC (e,u) TOP FOUND ! ("<<index_lepTop<<")"<<endl;}
        if(idx_W > 0) {cout<<"LEPTONIC (e,u) W BOSON FOUND ! ("<<idx_W<<")"<<endl;}
        if(index_hadTop > 0) {cout<<"HADRONIC TOP FOUND ! ("<<index_hadTop<<")"<<endl;}
        cout<<endl<<endl<<FMAG("========================")<<endl;
    }

    Fill_HighLevel_Variables(Zboson, RecoZ, lepTop, hadTop, TopZsystem, recoilQuark, lepZ1, lepZ2, lepTopl, lepTopnu, lepTopb, lepTopW, hadTopq1, hadTopq2, hadTopb, lepZ1_id, lepZ2_id, lepTopl_id);
//--------------------------------------------

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


 //  ####  ###### #    #      # ###### #####  ####
 // #    # #      ##   #      # #        #   #
 // #      #####  # #  #      # #####    #    ####
 // #  ### #      #  # #      # #        #        #
 // #    # #      #   ## #    # #        #   #    #
 //  ####  ###### #    #  ####  ######   #    ####

    if(genJetsHandle.isValid())
    {
        int nGenJet = genJetsHandle->size();
        for(int i1=0;i1<nGenJet;i1++)
        {
            const reco::GenJet& genJet1 = genJetsHandle->at(i1);
            TLorentzVector j1; j1.SetPtEtaPhiE(genJet1.pt(), genJet1.eta(), genJet1.phi(), genJet1.energy());

            if(j1.Pt() < 20) {continue;}
            if(j1.Eta() > 5.0) {continue;}

            // cout<<"jet "<<i1<<endl;
            // cout<<"pt "<<j1.Pt()<<endl;

            genJetsPt_.push_back(j1.Pt());
            genJetsEta_.push_back(j1.Eta());
            genJetsPhi_.push_back(j1.Phi());
            genJetsMass_.push_back(j1.M());

            ptHadSum_+= j1.Pt();
            njets_++;

            if(abs(j1.Eta()) > maxEtaJet_) {maxEtaJet_ = abs(j1.Eta());}

            if(LepTop_pt_ > 0 && lepTop.DeltaR(j1) < dR_tClosestJet_) {dR_tClosestJet_ = lepTop.DeltaR(j1);}

            for(int i2=i1+1;i2<nGenJet;i2++)
            {
                const reco::GenJet& genJet2 = genJetsHandle->at(i2);
                TLorentzVector j2; j2.SetPtEtaPhiE(genJet2.pt(), genJet2.eta(), genJet2.phi(), genJet2.energy());

                double pt_tmp = (j1+j2).Pt();
                double m_tmp = (j1+j2).M();
                double dEta_tmp = abs(j1.Eta() - j2.Eta());
                double dPhi_tmp = abs(j1.DeltaPhi(j2));
                double dR_tmp = j1.DeltaR(j2);
                if(pt_tmp > maxDiJet_pt_) {maxDiJet_pt_ = pt_tmp;}
                if(pt_tmp < minDiJet_pt_ || minDiJet_pt_==DEFVAL) {minDiJet_pt_ = pt_tmp;}
                if(m_tmp > maxDiJet_m_) {maxDiJet_m_ = m_tmp;}
                if(m_tmp < minDiJet_m_ || minDiJet_pt_==DEFVAL) {minDiJet_m_ = m_tmp;}

                if(dEta_tmp > maxDiJet_dEta_) {maxDiJet_dEta_ = dEta_tmp;}
                if(dEta_tmp < minDiJet_dEta_) {minDiJet_dEta_ = dEta_tmp;}
                if(dPhi_tmp > maxDiJet_dPhi_) {maxDiJet_dPhi_ = dPhi_tmp;}
                if(dPhi_tmp < minDiJet_dPhi_) {minDiJet_dPhi_ = dPhi_tmp;}
                if(dR_tmp > maxDiJet_dR_) {maxDiJet_dR_ = dR_tmp;}
                if(dR_tmp < minDiJet_dR_) {minDiJet_dR_ = dR_tmp;}
            }
        }
    } //genJetsHandle.isValid()

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
 * Return the mother genParticle (and make sure it has a different ID than the daughter <-> 'true mother')
 */
const reco::GenParticle* GenAnalyzer::getTrueMother(const reco::GenParticle part)
{
    const reco::GenParticle *mom = &part; //Start from daughter

    while(mom->numberOfMothers() > 0) //Loop recursively on particles mothers
    {
        for(unsigned int j=0; j<mom->numberOfMothers(); ++j)
        {
            mom = dynamic_cast<const reco::GenParticle*>(mom->mother(j));

            if(mom->pdgId() != part.pdgId()) //Stop once a mother is found which is different from the daughter passed in arg
            {
                return mom; //Return the mother particle
            }
        }
    }

    return mom;
}

//Look for particle's mother, and perform cone-matching to find index within gen collection
int GenAnalyzer::getTrueMotherIndex(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    int mother_index = -1;

    const reco::GenParticle* mom = getTrueMother(p); //Get particle's mother genParticle

    // while(p.pdgId() == mom->pdgId()) {mom = getTrueMother(*mom);} //Loop on mothers until a particle with different ID is found //already done in getTrueMother()

    //To find the index of the mother within the genPart collection, run on entire collection and perform dR-matching
    for(reco::GenParticleCollection::const_iterator genParticleSrc_m = genParticlesHandle->begin(); genParticleSrc_m != genParticlesHandle->end(); genParticleSrc_m++)
    {
        // cout<<genParticleSrc_m-genParticlesHandle->begin()<<endl; //Access iterator index
        reco::GenParticle *mcp_m = &(const_cast<reco::GenParticle&>(*genParticleSrc_m));
        if(fabs(mcp_m->pt()-mom->pt()) < 10E-3 && fabs(mcp_m->eta()-mom->eta()) < 10E-3) {mother_index = genParticleSrc_m-genParticlesHandle->begin(); break; } //Cone matching
    }

    return mother_index;
}

//-- Get ID of mother particle
//NB : expensive to call this function several times, because each time it will call getTrueMotherIndex() ! Better to call getTrueMotherIndex() directly once
int GenAnalyzer::getTrueMotherId(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    int mother_index = getTrueMotherIndex(genParticlesHandle, p); //Get particle's mother index

    if(mother_index < 0) {return -1;}

    return (*genParticlesHandle)[mother_index].pdgId();
}

bool GenAnalyzer::isZDecayProduct(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p, int& index_Z)
{
    int mom_index = getTrueMotherIndex(genParticlesHandle, p); //Get particle's mother index
    int mom_id = (*genParticlesHandle)[mom_index].pdgId();

    if(abs(mom_id) == 23) {index_Z = mom_index; return true;} //Z daughter found

    //Look for chain decay daughter (Z -> tau tau -> xxx)
    if(abs(mom_id) == 15)
    {
        const reco::GenParticle* mom = getTrueMother(p); //Get the mother (tau) particle
        int momTau_id = getTrueMotherId(genParticlesHandle, *mom); //Get the id of the tau's mother
        if(abs(momTau_id) == 23) {return true;} //Check if the tau's mother is itself a Z; could also be a Higgs, ...
    }

    return false;
}

//If final-state ele/mu is not from W/Z decay, must be part of the non-resonant pair production... or it could come from e.g. Z->tautau
bool GenAnalyzer::isFinalStateElMu(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    int mother_id = getTrueMotherId(genParticlesHandle, p);
    if((p.isPromptFinalState() || p.isDirectPromptTauDecayProductFinalState()) && isEleMu(p.pdgId()) && abs(mother_id) !=  24 && abs(mother_id) !=  23) //Final-state e,mu not coming from top or Z decay
    {
        // const reco::GenParticle* mom = getTrueMother(p); //Get the mother (tau) particle
        // int mom_mom_id = getTrueMotherId(genParticlesHandle, *mom); //Get the id of the tau's mother
        // if(abs(mom_mom_id) != 15)
        // {
        //     what is usually the mother for nonresonant ll prod ?
        //     cout<<"NON RESONANT LEPTON ?"<<endl;
        //     cout<<"Mother id --> "<<mother_id<<endl;
        //     cout<<"Mother's mother's id --> "<<mom_mom_id<<endl;
        // }
        return true;
    }

    return false;
}

//Return indicates whether the decay product is from a top quark
int GenAnalyzer::isTopDecayProduct(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p)
{
    int index_top=-1;

    int mom_id = getTrueMotherId(genParticlesHandle, p);

    if(abs(mom_id) == 6) {index_top = getTrueMotherIndex(genParticlesHandle, p);} //Top daughter found (t -> Wb)

    const reco::GenParticle* mom = getTrueMother(p);

    //Look for chain decay daughter (t -> W -> xx)
    if(abs(mom_id) == 24)
    {
        //Check id of the mother of the mother (W)
        int momW_id = getTrueMotherId(genParticlesHandle, *mom);

        if(abs(momW_id) == 6) {index_top = getTrueMotherIndex(genParticlesHandle, *mom);} //Top daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
    }

    //Look for chain decay daughter (t -> W -> tau -> e,u)
    if(abs(mom_id) == 15)
    {
        //Check id of the mother of the mother (tau)
        int momTau_id = getTrueMotherId(genParticlesHandle, *mom);

        if(abs(momTau_id) == 24)
        {
            const reco::GenParticle* momTau = getTrueMother(*mom);

            //Check id of the mother of the mother (W) of the mother (tau)
            int momW_id = getTrueMotherId(genParticlesHandle, *momTau);

            if(abs(momW_id) == 6) {index_top = getTrueMotherIndex(genParticlesHandle, *mom);;} //Top daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
        }
    }

    return index_top;
}

//Return indicates whether the decay product is either from a top or an antitop
int GenAnalyzer::isTopOrAntitopDecayProduct(Handle<reco::GenParticleCollection> genParticlesHandle, const reco::GenParticle p, int& index_top, int& index_antitop)
{
    int mom_id = getTrueMotherId(genParticlesHandle, p);

    if(mom_id == 6) {index_top = getTrueMotherIndex(genParticlesHandle, p); return 1;} //Top daughter found (t -> Wb)
    if(mom_id == -6) {index_antitop = getTrueMotherIndex(genParticlesHandle, p); return -1;} //AntiTop daughter found (t -> Wb)

    const reco::GenParticle* mom = getTrueMother(p);

    //Look for chain decay daughter (t -> W -> xx)
    if(abs(mom_id) == 24)
    {
        //Check id of the mother of the mother (W)
        int momW_id = getTrueMotherId(genParticlesHandle, *mom);

        if(momW_id == 6) {index_top = getTrueMotherIndex(genParticlesHandle, *mom); return 1;} //Top daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
        if(momW_id == -6) {index_antitop = getTrueMotherIndex(genParticlesHandle, *mom); return -1;} //AntiTop daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
    }

    //Look for chain decay daughter (t -> W -> tau -> e,u)
    if(abs(mom_id) == 15)
    {
        //Check id of the mother of the mother (tau)
        int momTau_id = getTrueMotherId(genParticlesHandle, *mom);

        if(abs(momTau_id) == 24)
        {
            const reco::GenParticle* momTau = getTrueMother(*mom);

            //Check id of the mother of the mother (W) of the mother (tau)
            int momW_id = getTrueMotherId(genParticlesHandle, *momTau);

            if(momW_id == 6) {index_top = getTrueMotherIndex(genParticlesHandle, *mom); return 1;} //Top daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
            if(momW_id == -6) {index_antitop = getTrueMotherIndex(genParticlesHandle, *mom); return -1;} //AntiTop daughter found (through W decay) //NB : pass the 'first mom' as arg <-> get second mom
        }
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

float GenAnalyzer::GetDeltaR(float eta1, float phi1, float eta2, float phi2)
{
   float DeltaPhi = TMath::Abs(phi2 - phi1);
   if(DeltaPhi > 3.141593 ) DeltaPhi -= 2.*3.141593;
   return TMath::Sqrt( (eta2-eta1)*(eta2-eta1) + DeltaPhi*DeltaPhi );
}

float GenAnalyzer::GetDeltaEta(TLorentzVector tlv1, TLorentzVector tlv2)
{
    float eta1 = tlv1.Eta();
    float eta2 = tlv2.Eta();

   return abs(eta2-eta1);
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

TLorentzVector GenAnalyzer::GetTLorentzVector_fromPartIndex(Handle<reco::GenParticleCollection> genParticlesHandle, int index)
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

// Top quark polarization angle theta*_pol (see TOP-17-023)
//From Potato code : tzq/include/Observable.h
//cos(top polarization angle) = ( vec(x) ) / ( norm(x) ), with x=P(q')[top].P(l)[top] and [top] means 'in top rest frame'. q' is the spectator (recoil) quark, and l is the top decay lepton.
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

    if(!spec.Vect().Mag() || !lep.Vect().Mag()) {return -9;}

    return (spec.Vect() * lep.Vect()) / (spec.Vect().Mag() * lep.Vect().Mag());
}

//Z boson polarization angle (see TOP-18-009)
//Cosine of the angle of the negatively charged lepton in the Z boson rest frame
//Alternative formula in Potato code : .../ttz3l/include/CosThetaStar.h
double GenAnalyzer::Compute_cosThetaStarPol_Z(TLorentzVector Zreco, TLorentzVector Zl_neg)
{
    const TVector3 lvBoost = Zreco.BoostVector();
    Zl_neg.Boost(-lvBoost);

    return (lvBoost * Zl_neg.Vect()) / (lvBoost.Mag() * Zl_neg.Vect().Mag());
}

void GenAnalyzer::Fill_HighLevel_Variables(TLorentzVector Zboson, TLorentzVector RecoZ, TLorentzVector lepTop, TLorentzVector hadTop, TLorentzVector TopZsystem, TLorentzVector recoilQuark, TLorentzVector lepZ1, TLorentzVector lepZ2, TLorentzVector lepTopl, TLorentzVector lepTopnu, TLorentzVector lepTopb, TLorentzVector lepTopW, TLorentzVector hadTopq1, TLorentzVector hadTopq2, TLorentzVector hadTopb, int lepZ1_id, int lepZ2_id, int lepTopl_id)
{
    if(debug) {cout<<"Fill_HighLevel_Variables()"<<endl;}

    // transverse W mass on the raw met and thierd lepton
    // mTW_ = std::sqrt(std::pow(lepTopl.Pt()+lepTopnu.Pt(),2) - std::pow(lepTopl.Px()+lepTopnu.Px(),2) - std::pow(lepTopl.Py()+lepTopnu.Py(),2)); //Bugged ?
    // delRZl_ = GetDeltaR(RecoZ,lepTopl);
    // Top_delRbl_ = GetDeltaR(lepTopl,lepTopb);
    // Top_delRbW_ = GetDeltaR(lepTopb,lepTopW);
    // delRtZ_      = GetDeltaR(lepTop,RecoZ);
    // delRtRecoilJet_ = GetDeltaR(lepTop,recoilQuark);
    // delRbRecoilJet_ = GetDeltaR(lepTopb,recoilQuark);
    // delRlRecoilJet_ = GetDeltaR(lepTopl,recoilQuark);
    // dEtaRecoilJetBJet_ = abs(recoilQuark.Eta() - lepTopb.Eta());
    // delRlWb_ = GetDeltaR(lepTopl, lepTopb);

    Mass_3l_ = (RecoZ + lepTopl).M();
    leptonCharge_ = (lepTopl_id > 0 ? 1:-1) + (lepZ1_id > 0 ? 1:-1) + (lepZ2_id > 0 ? 1:-1);
    lepAsym_ = -abs(lepTop.Eta()) * lepTopl_id/abs(lepTopl_id);

    dR_tZ_ = lepTop.DeltaR(Zboson);
    dR_ZlW_ = lepTopl.DeltaR(Zboson);
    dR_blW_ = lepTopl.DeltaR(lepTopb);
    dR_bW_ = lepTopW.DeltaR(lepTopb);
    dR_tjprime_ = lepTop.DeltaR(Zboson);
    dEta_tjprime_ = GetDeltaEta(lepTop, recoilQuark);
    dR_bjprime_ = lepTopb.DeltaR(recoilQuark);
    dEta_bjprime_ = GetDeltaEta(lepTopb, recoilQuark);
    dR_lWjprime_ = lepTopl.DeltaR(recoilQuark);
    dEta_lWjprime_ = GetDeltaEta(lepTopl, recoilQuark);
    dR_Zjprime_ = Zboson.DeltaR(recoilQuark);
    dEta_Zjprime_ = GetDeltaEta(Zboson, recoilQuark);

    //Max lep-X combination
    if(abs(lepZ1.DeltaPhi(lepZ2)) > maxDelPhiLL_) {maxDelPhiLL_ = abs(lepZ1.DeltaPhi(lepZ2));}
    if(abs(lepZ1.DeltaPhi(lepTopl)) > maxDelPhiLL_) {maxDelPhiLL_ = abs(lepZ1.DeltaPhi(lepTopl));}
    if(abs(lepZ2.DeltaPhi(lepTopl)) > maxDelPhiLL_) {maxDelPhiLL_ = abs(lepZ2.DeltaPhi(lepTopl));}

    if(lepZ1.DeltaR(lepTopb) > maxDelPhiLL_) {maxDelRbL_ = lepZ1.DeltaR(lepTopb);}
    if(lepZ2.DeltaR(lepTopb) > maxDelPhiLL_) {maxDelRbL_ = lepZ2.DeltaR(lepTopb);}
    if(lepTopl.DeltaR(lepTopb) > maxDelPhiLL_) {maxDelRbL_ = lepTopl.DeltaR(lepTopb);}

    //TTbar system (for ttZ)
    if(lepTop.Pt()>0 && hadTop.Pt()>0) {dR_ttbar_ = lepTop.DeltaR(hadTop); dEta_ttbar_ = GetDeltaEta(lepTop, hadTop);}

    //Closest lepton
    if(lepTopl.DeltaR(lepTop) < dR_tClosestLep_) {dR_tClosestLep_ = lepTopl.DeltaR(lepTop);}
    if(lepZ1.DeltaR(lepTop) < dR_tClosestLep_) {dR_tClosestLep_ = lepZ1.DeltaR(lepTop);}
    if(lepZ2.DeltaR(lepTop) < dR_tClosestLep_) {dR_tClosestLep_ = lepZ2.DeltaR(lepTop);}
    if(lepTop.DeltaR(recoilQuark) < dR_jprimeClosestLep_) {dR_jprimeClosestLep_ = lepTop.DeltaR(recoilQuark);}
    if(lepZ1.DeltaR(recoilQuark) < dR_jprimeClosestLep_) {dR_jprimeClosestLep_ = lepZ1.DeltaR(recoilQuark);}
    if(lepZ2.DeltaR(recoilQuark) < dR_jprimeClosestLep_) {dR_jprimeClosestLep_ = lepZ2.DeltaR(recoilQuark);}
    if(GetDeltaEta(lepTop, recoilQuark) < dEta_jprimeClosestLep_) {dEta_jprimeClosestLep_ = GetDeltaEta(lepTop, recoilQuark);}
    if(GetDeltaEta(lepZ1, recoilQuark) < dEta_jprimeClosestLep_) {dEta_jprimeClosestLep_ = GetDeltaEta(lepZ1, recoilQuark);}
    if(GetDeltaEta(lepZ2, recoilQuark) < dEta_jprimeClosestLep_) {dEta_jprimeClosestLep_ = GetDeltaEta(lepZ2, recoilQuark);}

    //Lepton channel
    if(abs(lepTopl_id)+abs(lepZ1_id)+abs(lepZ2_id) == 39) {channel_ = 0;} //uuu
    else if(abs(lepTopl_id)+abs(lepZ1_id)+abs(lepZ2_id) == 37) {channel_ = 1;} //uue
    else if(abs(lepTopl_id)+abs(lepZ1_id)+abs(lepZ2_id) == 35) {channel_ = 2;} //eeu
    else if(abs(lepTopl_id)+abs(lepZ1_id)+abs(lepZ2_id) == 33) {channel_ = 3;} //eee

    ptLepSum_ = (lepTop+lepZ1+lepZ2).Pt();
    mHT_ = ptLepSum_ + ptHadSum_;

    //need to specify negatively charged lepton, cf formula
    if(lepZ1_id < 0) {cosThetaStarPol_Z_ = Compute_cosThetaStarPol_Z(RecoZ, lepZ1);}
    else if(lepZ2_id < 0) {cosThetaStarPol_Z_ = Compute_cosThetaStarPol_Z(RecoZ, lepZ2);}
    cosThetaStarPol_Top_ = Compute_cosThetaStarPol_Top(lepTop, lepTopl, recoilQuark);
    
    lepTopnu_pt_ = lepTopnu.Pt();

    return;
}

bool GenAnalyzer::Event_Selection(int nEleMuFinalState, bool hasLepTop, bool hasHadTop, bool hasZDecayEMU, TLorentzVector lepZ1, TLorentzVector lepZ2, int lepZ1_id, int lepZ2_id)
{
    if(nEleMuFinalState < 3) {return false;} //Only consider events with >=3 ele or mu in FS

    //Only keep events with 3 e,mu ?
    //NB : events may be rejected because they contain hadronic taus, due to pt cuts, ...
    if(debug)
    {
        cout<<endl<<FRED("hasZDecayEMU "<<hasZDecayEMU<<"")<<endl;
        cout<<FRED("hasLepTop "<<hasLepTop<<"")<<endl;
        cout<<FRED("hasHadTop "<<hasHadTop<<"")<<endl<<endl;

        cout<<"lepZ1.Pt() "<<lepZ1.Pt()<<" / lepZ1_id "<<lepZ1_id<<endl;
        cout<<"lepZ2.Pt() "<<lepZ2.Pt()<<" / lepZ2_id "<<lepZ2_id<<endl;
        cout<<"abs((lepZ1+lepZ2).M()) "<<abs((lepZ1+lepZ2).M())<<endl;
    }

    if(!hasZDecayEMU || !hasLepTop) {return false;}

    return true;
}

//Get other decay products from leptonic top quark (NB: lepton already searched for)
void GenAnalyzer::Get_OtherDecayProducts_LepTop(Handle<reco::GenParticleCollection> genParticlesHandle, int index_lepTop, TLorentzVector& tlv_nu, TLorentzVector& tlv_b, TLorentzVector& tlv_W, int& idx_W)
{
    if(index_lepTop < 0) {return;}

    const GenParticle& top = (*genParticlesHandle)[index_lepTop];

    //Get vector of daughters' indices
    std::vector<int> daughter_indices = GetVectorDaughterIndices(genParticlesHandle, top);

    for(unsigned int idaughter=0; idaughter<daughter_indices.size(); idaughter++)
    {
        int idx = daughter_indices[idaughter];

        const GenParticle & daughter = (*genParticlesHandle)[idx];

        //if(abs(daughter.pdgId()) == 12 || abs(daughter.pdgId()) == 14 || abs(daughter.pdgId()) == 16) {tlv_nu = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx);}
        if(abs(daughter.pdgId()) == 5) {tlv_b = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx);}
        else if(abs(daughter.pdgId()) == 24)
        {
            idx_W = idx;
            tlv_W = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx_W);

            std::vector<int> daughter_indices_W = GetVectorDaughterIndices(genParticlesHandle, daughter);

            for(unsigned int idaughterW=0; idaughterW<daughter_indices_W.size(); idaughterW++)
            {
                int idx_W = daughter_indices_W[idaughterW];

		        const GenParticle & daughter_W = (*genParticlesHandle)[idx_W];

                if(abs(daughter_W.pdgId()) == 12 || abs(daughter_W.pdgId()) == 14 || abs(daughter_W.pdgId()) == 16) {tlv_nu = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx_W);}
            }
        }
        // cout<<"...Daughter index = "<<daughter_indices[idaughter]<<endl;
        // cout<<"... daughter ID = "<<daughter.pdgId()<<" (idx "<<daughter_indices[idaughter]<<")"<<endl;
    }
    
    if(debug)
    {
		cout<<"LepTop b pt = "<<tlv_b.Pt()<<endl;
		cout<<"LepTop W pt = "<<tlv_W.Pt()<<endl;
		cout<<"LepTop nu pt = "<<tlv_nu.Pt()<<endl;
	}

    return;
}

void GenAnalyzer::Get_OtherDecayProducts_HadTop(Handle<reco::GenParticleCollection> genParticlesHandle, int index_hadTop, TLorentzVector& tlv_q1, TLorentzVector& tlv_q2, TLorentzVector& tlv_b)
{
    if(index_hadTop < 0) {return;}

    const GenParticle& top = (*genParticlesHandle)[index_hadTop];

    //Get vector of daughters' indices
    std::vector<int> daughter_indices = GetVectorDaughterIndices(genParticlesHandle, top);

    for(unsigned int idaughter=0; idaughter<daughter_indices.size(); idaughter++)
    {
        int idx = daughter_indices[idaughter];

        const GenParticle & daughter = (*genParticlesHandle)[idx];

        if(abs(daughter.pdgId()) == 5) {tlv_b = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx);}
        else if(abs(daughter.pdgId()) == 24)
        {
            std::vector<int> daughter_indices_W = GetVectorDaughterIndices(genParticlesHandle, daughter);

            for(unsigned int idaughterW=0; idaughterW<daughter_indices_W.size(); idaughterW++)
            {
                int idx_W = daughter_indices_W[idaughterW];

                if(abs(daughter.pdgId()) < 5)
                {
                    if(tlv_q1.Pt()<=0) {tlv_q1 = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx_W);}
                    else {tlv_q2 = GetTLorentzVector_fromPartIndex(genParticlesHandle, idx_W);}
                }
            }
        }
    }

    if(debug)
    {
        cout<<"hadTop tlv_b.Pt() "<<tlv_b.Pt()<<endl;
        cout<<"hadTop tlv_q1.Pt() "<<tlv_q1.Pt()<<endl;
        cout<<"hadTop tlv_q2.Pt() "<<tlv_q2.Pt()<<endl;
    }

    return;
}


void GenAnalyzer::SetBranches(TTree* tree_)
{
    // tree_->Branch("pt"   , &genParticlesPt_   );
    // tree_->Branch("eta"  , &genParticlesEta_  );
    // tree_->Branch("phi"  , &genParticlesPhi_  );
    // tree_->Branch("mass" , &genParticlesMass_ );
    tree_->Branch("njets" , &njets_);
    tree_->Branch("genJetsPt" , &genJetsPt_);
    tree_->Branch("genJetsEta" , &genJetsEta_);
    tree_->Branch("genJetsPhi" , &genJetsPhi_);
    tree_->Branch("genJetsMass" , &genJetsMass_);


    tree_->Branch("v_weights" , &v_weights_);
    tree_->Branch("v_weightIds" , &v_weightIds_);

    tree_->Branch("mc_weight" , &mc_weight);
    tree_->Branch("mc_weight_originalValue" , &mc_weight_originalValue);
    tree_->Branch("originalXWGTUP" , &originalXWGTUP);

    tree_->Branch("cosThetaStarPol_Top" , &cosThetaStarPol_Top_);
    tree_->Branch("cosThetaStarPol_Z" , &cosThetaStarPol_Z_);

    tree_->Branch("Z_pt" , &Z_pt_);
    tree_->Branch("Z_eta" , &Z_eta_);
    tree_->Branch("Z_phi" , &Z_phi_);
    tree_->Branch("Z_m" , &Z_m_);
    tree_->Branch("Z_decayMode" , &Z_decayMode_);
    tree_->Branch("Zreco_pt" , &Zreco_pt_);
    tree_->Branch("Zreco_eta" , &Zreco_eta_);
    tree_->Branch("Zreco_phi" , &Zreco_phi_);
    tree_->Branch("Zreco_m" , &Zreco_m_);
    tree_->Branch("Zreco_dPhill" , &Zreco_dPhill_);
    tree_->Branch("LepTop_pt" , &LepTop_pt_);
    tree_->Branch("LepTop_eta" , &LepTop_eta_);
    tree_->Branch("LepTop_phi" , &LepTop_phi_);
    tree_->Branch("LepTop_m" , &LepTop_m_);
    tree_->Branch("TopZsystem_pt" , &TopZsystem_pt_);
    tree_->Branch("TopZsystem_eta" , &TopZsystem_eta_);
    tree_->Branch("TopZsystem_phi" , &TopZsystem_phi_);
    tree_->Branch("TopZsystem_m" , &TopZsystem_m_);
    tree_->Branch("LepTopLep_pt" , &LepTopLep_pt_);
    tree_->Branch("LepTopLep_eta" , &LepTopLep_eta_);
    tree_->Branch("LepTopLep_phi" , &LepTopLep_phi_);
    tree_->Branch("LepTopB_pt" , &LepTopB_pt_);
    tree_->Branch("LepTopB_eta" , &LepTopB_eta_);
    tree_->Branch("LepTopB_phi" , &LepTopB_phi_);
    tree_->Branch("recoilQuark_pt" , &recoilQuark_pt_);
    tree_->Branch("recoilQuark_eta" , &recoilQuark_eta_);
    tree_->Branch("recoilQuark_phi" , &recoilQuark_phi_);
    tree_->Branch("recoilQuark_id" , &recoilQuark_id);

    tree_->Branch("mTW" , &mTW_);
    tree_->Branch("Wlep_pt" , &Wlep_pt_);
    tree_->Branch("Wlep_eta" , &Wlep_eta_);
    tree_->Branch("Wlep_phi" , &Wlep_phi_);
    tree_->Branch("maxDiJet_pt" , &maxDiJet_pt_);
    tree_->Branch("maxDiJet_m" , &maxDiJet_m_);
    tree_->Branch("minDiJet_pt" , &minDiJet_pt_);
    tree_->Branch("minDiJet_m" , &minDiJet_m_);
    tree_->Branch("minDiJet_dEta" , &minDiJet_dEta_);
    tree_->Branch("maxDiJet_dEta" , &maxDiJet_dEta_);
    tree_->Branch("minDiJet_dPhi" , &minDiJet_dPhi_);
    tree_->Branch("maxDiJet_dPhi" , &maxDiJet_dPhi_);
    tree_->Branch("minDiJet_dR" , &minDiJet_dR_);
    tree_->Branch("maxDiJet_dR" , &maxDiJet_dR_);
    tree_->Branch("Mass_3l" , &Mass_3l_);
    tree_->Branch("leptonCharge" , &leptonCharge_);
    tree_->Branch("dR_tZ" , &dR_tZ_);
    tree_->Branch("dR_ZlW" , &dR_ZlW_);
    tree_->Branch("dR_blW" , &dR_blW_);
    tree_->Branch("dR_bW" , &dR_bW_);
    tree_->Branch("dR_tClosestLep" , &dR_tClosestLep_);
    tree_->Branch("dR_jprimeClosestLep" , &dR_jprimeClosestLep_);
    tree_->Branch("dEta_jprimeClosestLep" , &dEta_jprimeClosestLep_);
    tree_->Branch("dR_tjprime" , &dR_tjprime_);
    tree_->Branch("dEta_tjprime" , &dEta_tjprime_);
    tree_->Branch("dR_bjprime" , &dR_bjprime_);
    tree_->Branch("dEta_bjprime" , &dEta_bjprime_);
    tree_->Branch("dR_lWjprime" , &dR_lWjprime_);
    tree_->Branch("dEta_lWjprime" , &dEta_lWjprime_);
    tree_->Branch("dR_Zjprime" , &dR_Zjprime_);
    tree_->Branch("dEta_Zjprime" , &dEta_Zjprime_);
    tree_->Branch("maxEtaJet" , &maxEtaJet_);
    tree_->Branch("maxDelPhiLL" , &maxDelPhiLL_);
    tree_->Branch("maxDelRbL" , &maxDelRbL_);
    tree_->Branch("dR_tClosestJet" , &dR_tClosestJet_);
    tree_->Branch("dR_ttbar" , &dR_ttbar_);
    tree_->Branch("dEta_ttbar" , &dEta_ttbar_);
    tree_->Branch("channel" , &channel_);
    tree_->Branch("ptLepSum" , &ptLepSum_);
    tree_->Branch("ptHadSum" , &ptHadSum_);
    tree_->Branch("mHT" , &mHT_);
    tree_->Branch("lepAsym" , &lepAsym_);
    
    //NEW
    tree_->Branch("lepZ1_pt" , &lepZ1_pt_);
    tree_->Branch("lepZ2_pt" , &lepZ2_pt_);
    tree_->Branch("lepTopnu_pt" , &lepTopnu_pt_);
        
    return;
}

void GenAnalyzer::Init_Variables()
{
    // genParticlesPt_.clear();
    // genParticlesEta_.clear();
    // genParticlesPhi_.clear();
    // genParticlesMass_.clear();

    genJetsPt_.clear();
    genJetsEta_.clear();
    genJetsPhi_.clear();
    genJetsMass_.clear();

    // index_Z = -1;
    Z_pt_ = DEFVAL;
    Z_eta_ = DEFVAL;
    Z_phi_ = DEFVAL;
    Z_m_ = DEFVAL;
    Z_decayMode_ = DEFVAL;
    Zreco_pt_ = DEFVAL;
    Zreco_eta_ = DEFVAL;
    Zreco_phi_ = DEFVAL;
    Zreco_m_ = DEFVAL;
    Zreco_dPhill_ = DEFVAL;

    LepTop_pt_ = DEFVAL;
    LepTop_eta_ = DEFVAL;
    LepTop_phi_ = DEFVAL;
    LepTop_m_ = DEFVAL;

    LepTopLep_pt_ = DEFVAL;
    LepTopLep_eta_ = DEFVAL;
    LepTopLep_phi_ = DEFVAL;

    LepTopB_pt_ = DEFVAL;
    LepTopB_eta_ = DEFVAL;
    LepTopB_phi_ = DEFVAL;

    // index_top = -1;
    // Top_pt_ = DEFVAL;
    // Top_eta_ = DEFVAL;
    // Top_phi_ = DEFVAL;
    // Top_m_ = DEFVAL;

    // index_antitop = -1;
    // AntiTop_pt_ = DEFVAL;
    // AntiTop_eta_ = DEFVAL;
    // AntiTop_phi_ = DEFVAL;
    // AntiTop_m_ = DEFVAL;
    // LeadingTop_pt_ = DEFVAL;
    // LeadingTop_eta_ = DEFVAL;
    // LeadingTop_phi_ = DEFVAL;
    // LeadingTop_m_ = DEFVAL;

    TopZsystem_pt_ = DEFVAL;
    TopZsystem_eta_ = DEFVAL;
    TopZsystem_phi_ = DEFVAL;
    TopZsystem_m_ = DEFVAL;

    recoilQuark_pt_ = DEFVAL;
    recoilQuark_eta_ = DEFVAL;
    recoilQuark_phi_ = DEFVAL;
    recoilQuark_id = DEFVAL;

    mTW_ = DEFVAL;
    Wlep_pt_ = DEFVAL;
    Wlep_eta_ = DEFVAL;
    Wlep_phi_ = DEFVAL;

    cosThetaStarPol_Top_ = DEFVAL;
    cosThetaStarPol_Z_ = DEFVAL;

    // delRZl_ = DEFVAL;
    // Top_delRbl_ = DEFVAL;
    // Top_delRbW_ = DEFVAL;
    // delRtZ_ = DEFVAL;
    // delRtRecoilJet_ = DEFVAL;
    // delRbRecoilJet_ = DEFVAL;
    // delRlRecoilJet_ = DEFVAL;
    // dEtaRecoilJetBJet_ = DEFVAL;
    // delRlWb_ = DEFVAL;

    Mass_3l_ = DEFVAL;
    leptonCharge_ = DEFVAL;
    channel_ = DEFVAL;
    ptLepSum_ = DEFVAL;
    ptHadSum_ = DEFVAL;
    mHT_ = DEFVAL;
    lepAsym_ = DEFVAL;
    njets_ = 0;
    
    lepTopnu_pt_ = 0;
    lepZ1_pt_ = 0;
	lepZ2_pt_ = 0;

    dR_tZ_ = DEFVAL;
    dR_ZlW_ = DEFVAL;
    dR_blW_ = DEFVAL;
    dR_bW_ = DEFVAL;
    dR_tClosestLep_ = -DEFVAL;
    dR_jprimeClosestLep_ = -DEFVAL;
    dEta_jprimeClosestLep_ = -DEFVAL;
    dR_tClosestJet_ = -DEFVAL;
    dR_tjprime_ = DEFVAL;
    dEta_tjprime_ = DEFVAL;
    dR_bjprime_ = DEFVAL;
    dEta_bjprime_ = DEFVAL;
    dR_lWjprime_ = DEFVAL;
    dEta_lWjprime_ = DEFVAL;
    dR_Zjprime_ = DEFVAL;
    dEta_Zjprime_ = DEFVAL;
    maxEtaJet_ = DEFVAL;
    dR_ttbar_ = DEFVAL;
    dEta_ttbar_ = DEFVAL;
    maxDelPhiLL_ = DEFVAL;
    maxDelRbL_ = DEFVAL;

    maxDiJet_pt_ = DEFVAL;
    maxDiJet_m_ = DEFVAL;
    maxDiJet_dEta_ = DEFVAL;
    maxDiJet_dPhi_ = DEFVAL;
    maxDiJet_dR_ = DEFVAL;
    minDiJet_pt_ = DEFVAL;
    minDiJet_m_ = DEFVAL;
    minDiJet_dEta_ = -DEFVAL;
    minDiJet_dPhi_ = -DEFVAL;
    minDiJet_dR_ = -DEFVAL;

    v_weights_.clear();
    v_weightIds_.clear();

    mc_weight = 0;
    mc_weight_originalValue = 0;
    originalXWGTUP = 0;

    index_Higgs = -1;

    return;
}

//define this as a plug-in
DEFINE_FWK_MODULE(GenAnalyzer);
