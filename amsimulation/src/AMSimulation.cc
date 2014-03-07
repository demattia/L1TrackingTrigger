#include "AMSimulationFunctions.h"

using namespace std;

/**
   \mainpage
   \section build_sec Building the project
   In order to build the project, you will need:
   - Root (http://root.cern.ch) installed and configured ($ROOTSYS must be pointing on the installation directory and $ROOTSYS/bin must be in the PATH)
   - Boost (http://www.boost.org/) libraries and header files
   
   Then from the main directory (above ./src) you only need to type :
   \code
   make
   \endcode

   If everything goes fine, you should get a binary file called "AMSimulation".
 
   \section use_sec Using the program
   \subsection help Informations on how to use the program
   To get the list of options and parameters :
   \code
   ./AMSimulation --help
   \endcode
   \subsection generate Generating a pattern bank
   To generate a patterns bank, use the command :
   \code
   ./AMSimulation --generateBank
   \endcode
   All options can be stored in a file called amsimulation.cfg, here is an example :
   \code
   # Number of strips in a superstrip {16,32,64,128,256,512,1024}
   ss_size=32
   # Number of DC bits to use [0-3]
   dc_bits=3
   # Minimal PT of tracks used to generate a pattern
   pt_min=2
   # Maximal PT of tracks used to generate a pattern
   pt_max=10
   # Minimal ETA of tracks used to generate a pattern
   eta_min=-0.125
   # Maximal ETA of tracks used to generate a pattern
   eta_max=1.375
   # Directory containing root files with single muon/antimuon events (local or RFIO)
   input_directory=rfio:/my/rfio/directory/
   # Output file name
   bank_name=testOutput.pbk
   # Coverage [0-1]
   coverage=0.9
   # Root file containing sectors definitions
   sector_file=sec_test.root
   # Index of the sector to be used
   sector_id=17
   # Layers used
   active_layers=6 7 9 10
   \endcode

   Each option contained in the configuration file can be overwritten via the command line, for example :
   \code
   ./AMSimulation --generateBank --ss_size=64
   \endcode
   will set the value of the ss_size option to 64, whatever is contained in the configuration file.

   If you have created 2 banks for the same sector with 2 different PT range (2-10 GeV and 10-50 GeV for example), you can merge the 2 files into a single one using the command :
   \code
   ./AMSimulation --MergeBanks --inputFile testPT2-10.pbk --secondFile testPT10-50.pbk --outputFile testPT2-10-50.pbk
   \endcode
   \subsection find Finding patterns in events
   To search for patterns in events, enter :
   \code
   ./AMSimulation --findPatterns --inputFile <path to Root File containing events (local or RFIO)> --bankFile <path to your pattern bank file> --outputFile <Root output file> --ss_threshold <minimum number of stubs to activate the pattern> --startEvent <Index of first event to analyse> --stopEvent <Index of last event to analyse>
   \endcode

  The program is using a virtual detector during the search process. The geometry of this detector (layers, ladders, Z modules and strips per segment) is contained in the CMSPatternLayer class but can be overwritten with a detector.cfg file located in the root directory of the program. Here is an example of the syntax :
  \code
  #layerID,nb ladders,nb Z modules,nb strips per segment
  5,16,15,1024
  6,24,15,1024
  7,36,15,1024
  8,48,14,1024
  9,60,14,1024
  10,76,14,1024
  \endcode

   \subsection merge Merging result files
   To merge root files containing results from patterns recognition, you can use the hadd binary distributed with Root. This will work to merge results concerning the same sectors but different events.
   
   If you need to merge files containing the same events but different sectors you can use (NOT UP TO DATE!!):
   \code
   ./AMSimulation --MergeSectors --inputFile  <Root file for events A to X in sector 1> --secondFile <Root file for events A to X in sector 2> --outputFile <Resulting Root file for events A to X in sector 1 & 2>
   \endcode

   \subsection view Viewing the content of a pattern bank
   You can display the patterns contained in a patterns bank file using the command :
   \code
   ./AMSimulation --printBank --bankFile <You patterns bank file>
   \endcode
   If you are more interested in the encoding of the patterns you can display the patterns with a decimal representation of each pattern layer :
   \code
   ./AMSimulation --printBankBinary --bankFile <You patterns bank file>
   \endcode

   It should display one pattern per line.


   \author Guillaume Baulieu g.baulieu@ipnl.in2p3.fr
 **/

int main(int av, char** ac)
{
  namespace po = boost::program_options;
  po::variables_map vm;
  po::options_description desc("Allowed options");
  fillConfigurationFromFile(av, ac, desc, vm, "amsimulation.cfg");

  if (vm.count("help")) {
    cout << desc << "\n";

    std::exit(0);
  }

  if (vm.count("analyseBank")) {
    SectorTree st;
    {
      std::ifstream ifs(vm["inputFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st;
    }
    TFile f( vm["outputFile"].as<string>().c_str(), "recreate");
    createAnalysis(st);
  } else if (vm.count("generateBank")) {
    
    // vector<int> layers;
    SectorTree st;
    int stripSize=0;
    int dcBits=0;
    string partDirName="";
    string bankFileName="";
    string rootFileName="";
    string activeLayers = "";
    vector<int> active_layers;
    vector<int> forced_layers;
    vector<int> desactivated_layers;
    float threshold=0;
    float min=0;
    float max=0;
    float minEta=0;
    float maxEta=0;
    int maxNbFake=0;
    int sector_tklayout_id=0;
    map<int,pair<float,float> > eta = CMSPatternLayer::getLayerDefInEta();
    
    try{
      stripSize=vm["ss_size"].as<int>();
      cout<<"Superstrip size : "<<stripSize<<endl;
      dcBits=vm["dc_bits"].as<int>();
      cout<<"DC bits number : "<<dcBits<<endl;
      min=vm["pt_min"].as<int>();
      cout<<"PT min : "<<min<<endl;
      max=vm["pt_max"].as<int>();
      cout<<"PT max : "<<max<<endl;
      minEta=vm["eta_min"].as<float>();
      cout<<"ETA min : "<<minEta<<endl;
      maxEta=vm["eta_max"].as<float>();
      cout<<"ETA max : "<<maxEta<<endl;
      maxNbFake=vm["maxFakeSStrips"].as<int>();
      cout<<"Max number of fake superstrips : "<<maxNbFake<<endl;
      cout<<"Coverage : "<<vm["coverage"].as<float>()<<endl;
      partDirName=vm["input_directory"].as<string>();
      cout<<"Using particules from "<<partDirName<<endl;
      bankFileName=vm["bank_name"].as<string>();
      cout<<"Output file name is "<<bankFileName<<endl;
      activeLayers=vm["active_layers"].as<string>();
      cout<<"Using layers "<<activeLayers<<endl;
      sector_tklayout_id=vm["sector_id"].as<int>();
      cout<<"Using sector "<<sector_tklayout_id<<" from "<<vm["sector_file"].as<string>()<<endl;

      //Get the active/forced/inactive layers
      //layersWithoutSigns : list of all layers

      //forceLayers : layers prefixed with a '+' : no fake stub will be allowed
      //if there is no stub on this layer -> there will be no pattern

      //inactiveLayers : layers prefixed with a '-' : only fake stubs on this layer
      //even if there is a stub it will be replaced with a fake one

      string layersWithoutSigns = activeLayers;
      string forceLayers="";
      string inactiveLayers="";
      size_t found = layersWithoutSigns.find("+");
      while (found!=string::npos){
        layersWithoutSigns.erase(found,1);//remove the '+'
        size_t endIndex = layersWithoutSigns.find(" ", found);
        if(endIndex!=string::npos)
          endIndex=endIndex-found+1;
        forceLayers.append(layersWithoutSigns.substr(found,endIndex));//add the layer number and the following space
        found = layersWithoutSigns.find("+"); // search for the next '+'
      }
      found = layersWithoutSigns.find("-");
      while (found!=string::npos){
        layersWithoutSigns.erase(found,1);
        size_t endIndex = layersWithoutSigns.find(" ", found);
        if(endIndex!=string::npos)
          endIndex=endIndex-found+1;
        inactiveLayers.append(layersWithoutSigns.substr(found,endIndex));
        found = layersWithoutSigns.find("-");
      }

      std::istringstream is( layersWithoutSigns );
      int n;
      while( is >> n ) {
        active_layers.push_back(n);
      }
      std::istringstream fl( forceLayers );
      while( fl >> n ) {
        forced_layers.push_back(n);
      }
      std::istringstream il( inactiveLayers );
      while( il >> n ) {
        desactivated_layers.push_back(n);
      }

      //remove the force_layers from the eta list -> no fake stub will be added
      for(unsigned int i=0;i<forced_layers.size();i++){
        map<int,pair<float,float> >::iterator it = eta.find(forced_layers[i]);
        eta.erase(it);
      }

      //change the eta definition of the desactivated layers
      //they will not be reachable and a fake stub will be created
      for(unsigned int i=0;i<desactivated_layers.size();i++){
        eta[desactivated_layers[i]].first=10;
        eta[desactivated_layers[i]].second=10;
      }

      size_t end_index = bankFileName.find(".pbk");
      if(end_index==string::npos)
        end_index=bankFileName.length()-4;
      rootFileName = bankFileName.substr(0,end_index)+"_report.root";
      threshold=vm["coverage"].as<float>();
      createSectorFromRootFile(&st,vm["sector_file"].as<string>(), active_layers, sector_tklayout_id);
    }
    catch(boost::bad_any_cast e){
      cout<<"At least one option is missing! Please check : "<<endl;
      cout<<desc<<endl;
      return -1;
    }
    
    vector<Sector*> list = st.getAllSectors();
    cout<<"Sector :"<<endl;
    for(unsigned int i=0;i<list.size();i++){
      cout<<*list[i];
      cout<<endl;
    }

    if(active_layers.size()>8){
      cout<<"ERROR : your sector contains "<<active_layers.size()<<" layers : maximum number of layers is 8!"<<endl;
      return -1;
    }
    
    PatternGenerator pg(stripSize);//Super strip size
    pg.setLayers(active_layers);
    pg.setInactiveLayers(desactivated_layers);
    pg.setParticuleDirName(partDirName);
    pg.setMinPT(min);
    pg.setMaxPT(max);
    pg.setMinEta(minEta);
    pg.setMaxEta(maxEta);
    pg.setMaxFakeSuperstrips(maxNbFake);
    TFile f(rootFileName.c_str(), "recreate");
    pg.setVariableResolution(dcBits);
    pg.generate(&st, 40000, threshold, eta);


    if(pg.getVariableResolutionState()>0){
      cout<<"LD Patterns : "<<st.getLDPatternNumber()<<endl;
    }
  
    cout<<"Saving SectorTree...";
    {
      const SectorTree& ref = st;
      std::ofstream ofs(bankFileName.c_str());
      boost::archive::text_oarchive oa(ofs);
      oa << ref;
      cout<<"done."<<endl;
    }
    
  }
  else if(vm.count("testSectors")) {
    vector<int> layers;
    getLayers(layers);
    float phi0_min = getPhiMin();
    float phi0_max = getPhiMax();
    float eta_min = getEtaMin();
    float eta_max = getEtaMax();
    vector< vector<int> > restriction = getRestrictions(layers);
    string fn = getSectorDefFileName();
    createFromSimu(fn, layers, restriction, phi0_min, phi0_max, eta_min, eta_max);
  }
  else if(vm.count("decode")) {
    CMSPatternLayer p;
    int val = vm["decode"].as<int>();
    p.setIntValue(val);
    cout<<p.toString()<<endl;
  }
  else if(vm.count("findPatterns")) {
    SectorTree st;
    cout<<"Loading pattern bank..."<<endl;
    {
      std::ifstream ifs(vm["bankFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st;
    }

    ///////////////////////////////////////////////////////////////
    // If we don't have a fitter -> create a Hough default one
    vector<Sector*> sectors = st.getAllSectors();
    for(unsigned int i=0;i<sectors.size();i++){
      if(sectors[i]->getFitter()==NULL){
        //TrackFitter* fitter = new KarimakiTrackFitter(sectors[i]->getNbLayers());
        TrackFitter* fitter = new HoughFitter(sectors[i]->getNbLayers());
        sectors[i]->setFitter(fitter);
        sectors[i]->updateFitterPhiRotation();
      }
    }
    ///////////////////////////////////////////////////////////////

    PatternFinder pf(st.getSuperStripSize(), vm["ss_threshold"].as<int>(), &st,  vm["inputFile"].as<string>().c_str(),  vm["outputFile"].as<string>().c_str());
    {
      boost::progress_timer t;
      int start = vm["startEvent"].as<int>();
      int stop = vm["stopEvent"].as<int>();
      pf.find(start, stop);
      cout<<"Time used to analyse "<<stop-start+1<<" events : "<<endl;
    }
  }
  else if(vm.count("buildFitParams")) {
    SectorTree st;
    cout<<"Loading pattern bank..."<<endl;
    {
      std::ifstream ifs(vm["bankFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st;
    }

    map<int,pair<float,float> > eta_limits;// eta values for which each layer does exist
    eta_limits[6]=pair<float,float>(-1.69,1.69);
    eta_limits[7]=pair<float,float>(-1.41,1.41);
    eta_limits[8]=pair<float,float>(-1.19,1.19);
    eta_limits[9]=pair<float,float>(-1.02,1.02);
    eta_limits[10]=pair<float,float>(-0.87,0.87);
    eta_limits[11]=pair<float,float>(1.12,2.19);
    eta_limits[12]=pair<float,float>(1.19,2.19);
    eta_limits[13]=pair<float,float>(1.28,2.19);
    eta_limits[14]=pair<float,float>(1.35,2.19);
    eta_limits[15]=pair<float,float>(1.43,2.19);

    PrincipalFitGenerator pfg(vm["inputFile"].as<string>().c_str(), &st);
    pfg.generate(eta_limits, 2, 100, 0, 0.87);
    
    cout<<"Saving SectorTree...";
    {
      const SectorTree& ref = st;
      std::ofstream ofs(vm["outputFile"].as<string>().c_str());
      boost::archive::text_oarchive oa(ofs);
      oa << ref;
      cout<<"done."<<endl;
    }
    
  }
  else if(vm.count("printBank")) {
    SectorTree st;
    cout<<"Loading pattern bank..."<<endl;
    {
      std::ifstream ifs(vm["bankFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st;
    }
    vector<Sector*> sectors = st.getAllSectors();
    for(unsigned int i=0;i<sectors.size();i++){
      Sector* mySector = sectors[i];
      vector<GradedPattern*> patterns = mySector->getPatternTree()->getLDPatterns();
      for(unsigned int j=0;j<patterns.size();j++){
        Pattern* p = patterns[j];
        for(int k=0;k<p->getNbLayers();k++){
          PatternLayer* mp = p->getLayerStrip(k);
          cout<<((CMSPatternLayer*)mp)->toString()<<" - ";
        }
        cout<<endl;
      }
    }
  }
  else if(vm.count("printBankBinary")) {
    SectorTree st;
    cout<<"Loading pattern bank..."<<endl;
    {
      std::ifstream ifs(vm["bankFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st;
    }
    vector<Sector*> sectors = st.getAllSectors();
    for(unsigned int i=0;i<sectors.size();i++){
      Sector* mySector = sectors[i];
      vector<GradedPattern*> patterns = mySector->getPatternTree()->getLDPatterns();
      for(unsigned int j=0;j<patterns.size();j++){
        Pattern* p = patterns[j];
        for(int k=0;k<p->getNbLayers();k++){
	  PatternLayer* mp = p->getLayerStrip(k);
	  cout<<((CMSPatternLayer*)mp)->toStringBinary()<<" - ";
	}
        cout<<endl;
      }
    }
  }
  else if(vm.count("MergeSectors")) {
    PatternFinder::mergeFiles(vm["outputFile"].as<string>().c_str(), vm["inputFile"].as<string>().c_str(), vm["secondFile"].as<string>().c_str());
  }
  else if(vm.count("MergeBanks")) {
    SectorTree st1;
    cout<<"Loading pattern bank from "<<vm["inputFile"].as<string>().c_str()<<"..."<<endl;
    {
      std::ifstream ifs(vm["inputFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st1;
    }
    vector<Sector*> list1 = st1.getAllSectors();
    unsigned int nbSectors1 = list1.size();
    if(nbSectors1>1){
      cout<<"You can only merge banks containing 1 sector ("<<nbSectors1<<" found)"<<endl;
      return -1;
    }

    int nbPatterns1 = list1[0]->getPatternTree()->getLDPatternNumber();
    cout<<nbPatterns1<<" patterns found."<<endl;
    SectorTree st2;
    cout<<"Loading pattern bank from "<<vm["secondFile"].as<string>().c_str()<<"..."<<endl;
    {
      std::ifstream ifs(vm["secondFile"].as<string>().c_str());
      boost::archive::text_iarchive ia(ifs);
      ia >> st2;
    } 
    vector<Sector*> list2 = st2.getAllSectors();
    unsigned int nbSectors2 = list2.size();
    if(nbSectors2>1){
      cout<<"You can only merge banks containing 1 sector ("<<nbSectors2<<" found)"<<endl;
      return -1;
    }
    if(st1.getSuperStripSize()!=st2.getSuperStripSize()){
      cout<<"You can only merge banks using the same superstrip size ("<<st1.getSuperStripSize()<<" and "<<st2.getSuperStripSize()<<" found)"<<endl;
      return -1;
    }
    if(list1[0]->getNbLayers()!=list2[0]->getNbLayers()){
      cout<<"You can only merge banks using the same number of layers ("<<list1[0]->getNbLayers()<<" and "<<list2[0]->getNbLayers()<<" found)"<<endl;
      return -1;
    }
    int nbPatterns2 = list2[0]->getPatternTree()->getLDPatternNumber();
    cout<<nbPatterns2<<" patterns found."<<endl;

    cout<<"Merging banks..."<<endl;
    if(nbPatterns1>nbPatterns2){
      list1[0]->getPatternTree()->addPatternsFromTree(list2[0]->getPatternTree());
      cout<<"-> "<<list1[0]->getPatternTree()->getLDPatternNumber()<<" patterns."<<endl;
      cout<<"Saving new bank in "<<vm["outputFile"].as<string>().c_str()<<"..."<<endl;
      {
        const SectorTree& ref = st1;
        std::ofstream ofs(vm["outputFile"].as<string>().c_str());
        boost::archive::text_oarchive oa(ofs);
        oa << ref;
      }
    }
    else{
      list2[0]->getPatternTree()->addPatternsFromTree(list1[0]->getPatternTree());
      cout<<"-> "<<list2[0]->getPatternTree()->getLDPatternNumber()<<" patterns."<<endl;
      cout<<"Saving new bank in "<<vm["outputFile"].as<string>().c_str()<<"..."<<endl;
      {
        const SectorTree& ref = st2;
        std::ofstream ofs(vm["outputFile"].as<string>().c_str());
        boost::archive::text_oarchive oa(ofs);
        oa << ref;
      }
    }
  }
  else if(vm.count("testCode")) {
    //cout<<"Nothing to be done"<<endl;
   
    string result;
    {
      SectorTree sTest;
      cout<<"Loading pattern bank..."<<endl;
      {
        //std::ifstream ifs("/home/infor/baulieu/private/cms/CMSSW_6_1_2_SLHC3/src/amsimulation/test.pbk");
        std::ifstream ifs("/home/infor/baulieu/private/cms/CMSSW_6_1_2_SLHC3/src/amsimulation/612_SLHC6_MUBANK_lowmidhig_sec0_ss64_cov40.pbk");
        boost::archive::text_iarchive ia(ifs);
        ia >> sTest;
      }
      cout<<"Sector :"<<endl;
      cout<<*(sTest.getAllSectors()[0])<<endl;
      cout<<"loaded "<<sTest.getAllSectors()[0]->getLDPatternNumber()<<" patterns for sector "<<sTest.getAllSectors()[0]->getOfficialID()<<endl;
      /*
      cout<<"saving pattern bank..."<<endl;
      {
    const SectorTree& ref = sTest;
    cout<<"trying to save "<<sTest.getAllSectors()[0]->getLDPatternNumber()<<" patterns"<<endl;
    std::ofstream ofs("/home/infor/baulieu/private/cms/CMSSW_6_1_2_SLHC3/src/amsimulation/testOut.pbk");
    boost::archive::text_oarchive oa(ofs);
    oa << ref;
      }
      */
    }
  }
}
