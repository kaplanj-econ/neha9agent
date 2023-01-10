#include <iostream>
using namespace std;
#include "../headers/grove.hpp"
#include "../headers/commodity.hpp"
#include "../headers/behavior.hpp"
#include "../headers/coord.hpp"
#include "../headers/parameterSet.hpp"
#include "../headers/bioABM.h"
#include "../headers/groversbank.hpp"
#include <boost/algorithm/string.hpp>
#include <math.h>
#include <algorithm>

//#define SURVIVAL_DEBUG

#ifdef SURVIVAL_DEBUG
#define sdebug(x) cout << x
#else
#define sdebug(x) 
#endif

//Needed define-aheads
void Phase4();

/********************************************************************
* GLOBALS
*********************************************************************/
// Grove* agents;
Grove agents[ParameterSet::gridLength][ParameterSet::gridWidth];
Groversbank agentsinfo[ParameterSet::gridLength][ParameterSet::gridWidth];
Behavior* behaviorPatterns[3];
string harvestDays;
string yieldFilename;
double commodityPrice;
double commodityCost;
int commodityMaxAge;
ofstream outputFile;
string outputFilename;
string strategyParameters;
string strategyFlags; 
string agentsbankinfo;
string agencyFlags;
double memoryvalue;
int experimentID;
int memorylength = ParameterSet::parammemorylength;
double memorylengthyear[ParameterSet::parammemorylength];

boost::random::mt19937 econ_rng(std::time(0));
boost::random::uniform_01<> econ_gen;


/*****************************************************
 * String split
 * ***************************************************/
vector<string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

/**************************************************************
 * Configure initial behaviors
 * parses behavior parameter and populates behavior vector
 * ************************************************************/
void configureInitialBehaviors(Grove *g, string behaviorString) {
     //Configure initial behaviors
    vector<string> behavioridxs;
    boost::split(behavioridxs, behaviorString, boost::is_any_of(","));
    for (int i = 0; i < behavioridxs.size(); i++) {
        int idx = stoi(behavioridxs[i]);
        Behavior* b = behaviorPatterns[idx];
        g->behaviorPatterns.push_back(b);
    }
}


/**************************************************************
* Initialize CHMA 
* Take in our grove grid and uniform crop and populate the grid
*
* STRATEGY PARAMS SPEC
* 0: Rogue cost
* 1: Rogue frequency
* 2: Rogue radius
* *************************************************************/
void InitialiseCHMA(Commodity crop) {
   // agents = new Grove[ParameterSet::gridLength*ParameterSet::gridWidth];
    vector<string> sParams = split(strategyParameters, ";");
    vector<string> sFlags = split(strategyFlags, ";");
    vector<string> agencyParams = split(agencyFlags, ";");
    vector<string> sagentsbankinfo = split(agentsbankinfo,";");
   
    int latticeRows = bioABM::getNumRows();
    int latticeCols = bioABM::getRowLength();
    int cropRowSize = latticeRows / ParameterSet::gridLength;
    int cropColSize = latticeCols / ParameterSet::gridWidth;
    int k =0;   
    for (int i = 0; i < ParameterSet::gridLength; i++) {

         for(int j=0; j<ParameterSet::gridWidth;j++){
        //Create grove

                int i_lb = (i / ParameterSet::gridLength) * cropRowSize;
                int i_ub = i_lb + cropRowSize;
                int j_lb = (i % ParameterSet::gridLength) * cropColSize;
                int j_ub = j_lb + cropColSize;
                bool agency = (stoi(agencyParams[0]) == 1);


                 //Create and assign strategy vector
                vector<string> sFlags_agent = split(sFlags[k], ",");
                vector<string> sParams_agent = split(sParams[k], ",");
                vector<string> ssagentsbankinfo = split(sagentsbankinfo[k], ",");
                k = k+1;
              
                agents[i][j] = Grove(crop, agency, i_lb, i_ub, j_lb, j_ub);

            
                //Rogue trees
                if (stoi(sFlags_agent[0]) == 1) {
                   agentsinfo[i][j] = Groversbank(stod(ssagentsbankinfo[0]),stod(ssagentsbankinfo[1]),"Rogue");
                    agents[i][j].behaviorPatterns.push_back(new RogueTrees(stod(sParams_agent[0]),
                                                                        stod(sParams_agent[1]),
                                                                        stod(sParams_agent[2]),
                                                                        stod(sParams_agent[3]),
                                                                        stod(sParams_agent[4]))
                                                        );
                }

                    
                //Spraying
                if (stoi(sFlags_agent[1]) == 1) {
                   agentsinfo[i][j] = Groversbank(stod(ssagentsbankinfo[0]),stod(ssagentsbankinfo[1]),"Spray");
                    Behavior* spray = new SprayTrees(stod(sParams_agent[5]),
                                                    stod(sParams_agent[6]),
                                                    bioABM::getSpringStart(),
                                                    bioABM::getSummerStart(),
                                                    bioABM::getFallStart());
                    agents[i][j].behaviorPatterns.push_back(spray);
                }
                            
                //Denser planting
                if (stoi(sFlags_agent[2]) == 1) {
                  agentsinfo[i][j] = Groversbank(stod(ssagentsbankinfo[0]),stod(ssagentsbankinfo[1]),"DensePlanting");
                    Behavior * dPlant = new DensePlanting(
                        stod(sParams_agent[7]),
                        stod(sParams_agent[8])
                    );
                    agents[i][j].behaviorPatterns.push_back(dPlant);
                }

                if (stoi(sFlags_agent[3]) == 1) {
                   agentsinfo[i][j] = Groversbank(stod(ssagentsbankinfo[0]),stod(ssagentsbankinfo[1]),"RectangularRogue");
                    Behavior * wideRogue = new RectangularRogue(
                        stod(sParams_agent[9]),
                        stod(sParams_agent[10]),
                        stoi(sParams_agent[11]),
                        stoi(sParams_agent[12]),
                        stoi(sParams_agent[13]),
                        stod(sParams_agent[14])
                    );
                    agents[i][j].behaviorPatterns.push_back(wideRogue);
                }

              // cout << i << j << agentsinfo[i][j].getgroversbankprofit() << agentsinfo[i][j].getgroversbankhlbseverity() << agentsinfo[i][j].getgroversbankbehaviortype() << endl;

               // agentsinfo[i][j].setgroverbankparameters(stod(ssagentsbankinfo[0]),10,"Rogue");
        }
    }

      
    //Initial logging and planning
    for (int i = 0; i < ParameterSet::gridLength ; i++) { 
        for(int j=0; j<ParameterSet::gridWidth;j++){
            for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                agents[i][j].behaviorPatterns[k]->PlanActions();
            }
        }
    }
    return;
}




/*************************************************************
* Phase 1
* Psyllid growth, movement, and infection. All handled by the
* biological model
*************************************************************/
void Phase1() {
    bioABM::advanceBiologicalModel();
}

/*************************************************************
* Phase 2
* Execution of planned actions
************************************************************/
void Phase2() {
    int period_t = bioABM::getModelDay();
    for (int i = 0; i < ParameterSet::gridLength; i++) {
        for(int j=0; j<ParameterSet::gridWidth;j++){
            int relativePeriod = period_t % 365;
            for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                if (agents[i][j].behaviorPatterns[k]->actionPlannedOnDay(relativePeriod)) {
                    agents[i][j].behaviorPatterns[k]->executeAction(&agents[i][j]);
                }
                
            }
        }
    }
    //DEPRECATED
    /*
    for (int i = 0; i < ParameterSet::gridLength; i++) {
        for (int j = 0; j < ParameterSet::gridWidth; j++) {
            int relativePeriod = period_t % 365;
            for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                plan_func action = agents[i][j].behaviorPatterns[k]->getActionAtDay(relativePeriod);
                 if (action != NULL) {
                    action(&agents[i][j]);
                 }
            }
        }
    }*/
}

/*********************************************************
 * Weibull Survival
 * Returns the survival probability given 
 * vector of characteristsics
 * *******************************************************/
double weibullSurvival(int t, string strategy, double efficacy, string groveID, double alpha) {
    //cout << "Weibull(" << t << ", " << strategy << ", " << efficacy << ", " << groveID << ", " << alpha << "): ";
    double intercept = 6.345658;
    double indv_coef = 0.025527;
    double group_coef = 0.021299;
    double e75_coef = -0.000461;
    double e85_coef = 0.004604;
    double alpha1_coef = 0.396100;
    double g01_coef = -0.135702;
    double g02_coef = 0.001482;
    double g10_coef = -0.288761;
    double g11_coef = -0.510001;
    double g12_coef = -0.270179;
    double g20_coef = -0.082489;
    double g21_coef = -0.242598;
    double g22_coef = -0.067161;
    double threeterm_coef = 0.183670;
    double e75alpha1_coef = 0.279668;
    double e85alpha1_coef = 1.003784;
    double scale = 0.549;

    double lp_alpha0 = intercept;
    bool indv = (strategy == "Individual Action");
    bool group = (strategy == "Group Action");
    bool e75 = (efficacy == 0.75);
    bool e85 = (efficacy == 0.85);
    lp_alpha0 += (indv * indv_coef) + (group * group_coef) + (e75 * e75_coef) + (e85 * e85_coef);
    
    if (groveID == "g01") {
        lp_alpha0 += g01_coef;
    }
    else if (groveID == "g02") {
        lp_alpha0 += g02_coef;
    }
    else if (groveID == "g10") {
        lp_alpha0 += g10_coef;
    }
    else if (groveID == "g11") {
        lp_alpha0 += g11_coef;
    }
    else if (groveID == "g12") {
        lp_alpha0 += g12_coef;
    }
    else if (groveID == "g20") {
        lp_alpha0 += g20_coef;
    }
    else if (groveID == "g21") {
        lp_alpha0 += g21_coef;
    }
    else if (groveID == "g22") {
        lp_alpha0 += g22_coef;
    }
    
    double lp_alpha1 = lp_alpha0;
    bool threeTerm = (e85 && (indv || group));
    lp_alpha1 += alpha1_coef + (e75 * e75alpha1_coef) + (e85 * e85alpha1_coef) + (threeTerm * threeterm_coef);

    double survival_alpha0 = exp(-1 * pow(t / exp(lp_alpha0), 1/scale));
    double survival_alpha1 = exp(-1 * pow(t / exp(lp_alpha1), 1/scale));
    double scaled = survival_alpha0 + alpha*(survival_alpha1 - survival_alpha0);
    //cout << scaled << endl;
    return scaled;
}
/**********************************************************
* Get Expected Risk
* Calculates the growers expected risk of infection based on
* their expectation, the extension agents expectation, and
* their trust in the extension agent
* TEMPORARILY DISABLED FOR MANUSCRIPT 2
***********************************************************/
double getExpectedRisk(Grove*g, int growerI, int growerJ) {
    /*string growerID = string("g") + to_string(growerI) + to_string(growerJ);
    string behaviorName;
    if (g->getBehavior() == behaviorPatterns[0]) {
        behaviorName = "No Action";
    }
    else if (g->getBehavior() == behaviorPatterns[1]) {
        behaviorName = "Individual Action";
    }
    else {
        behaviorName = "Group Action";
    }
    double extensionExpectation = 1 - weibullSurvival(bioABM::getModelDay(), behaviorName, 
                                                  g->getSprayEfficacy(), growerID,
                                                  g->getAlpha());
    double adjustedExpectation = g->getLambda() * (extensionExpectation - g->lastAdjustedRisk) + g->lastAdjustedRisk;
    g->lastGrowerRisk = adjustedExpectation;
    g->lastExtensionRisk = extensionExpectation;
    g->lastAdjustedRisk = adjustedExpectation;
    return adjustedExpectation;*/
    return 0;
    
}


double getMeanHLB(Grove g) {
    int* ibounds = g.getIBounds();
    int* jbounds = g.getJBounds();
    double totalCells = 0.0;
    double totalHLB = 0.0;
    for (int i = ibounds[0]; i < ibounds[1]; i++) {
        for (int j = jbounds[0]; j < jbounds[1]; j++) {
            if (bioABM::isTreeAlive(i,j)) {
                totalCells += 1.0;
                totalHLB += bioABM::getSeverityAt(i, j);
            }
        }
    }
    if (totalCells > 0) {
        return totalHLB / totalCells;
    } else {
        return 0;
    }
    
}

int getDeadTrees(Grove g) {
    int* ibounds = g.getIBounds();
    int* jbounds = g.getJBounds();
    int deadTrees = 0;
    for (int i = ibounds[0]; i < ibounds[1]; i++) {
        for (int j = jbounds[0]; j < jbounds[1]; j++) {
            if (!bioABM::isTreeAlive(i,j)) {
                deadTrees++;
            }
        }
    }
    return deadTrees;

}
/********************************************************
* Phase 3
* Behavior Determination
* TEMPORARILY DISABLED DURING MANUSCRIPT 2 PLANNING
********************************************************/
void Phase3() {
    /*
    //Only determine during a planning period
    int period_t = bioABM::getModelDay();
    if ( (period_t % ParameterSet::planningLength) != 0 ) { return; }

    

    //Determine behavior for agents with agency
    for (int i = 0; i < ParameterSet::gridLength; i++) {
        for (int j = 0; j < ParameterSet::gridWidth; j++) {
            if (!agents[i][j].hasAgency()) { continue; }
            //Gather info for assessing risk
            //Assess risk
            double risk = getExpectedRisk(&agents[i][j], i, j);
            double meanHLB = getMeanHLB(agents[i][j]);
            bool findsHLB = econ_gen(econ_rng) <= meanHLB;
            if (findsHLB || agents[i][j].foundHLB) {
                risk = 1.0;
                if (!agents[i][j].foundHLB) {
                    agents[i][j].foundHLB = true;
                    agents[i][j].foundHLB_day = bioABM::getModelDay();
                }
            }
            agents[i][j].lastAdjustedRisk = risk;
            double maxExpectedValue = 0;
            int maxEVIndex = -1;
            int totalProjectingDays = ParameterSet::projectionLength + bioABM::getModelDuration();
            for (int k = 0; k < ParameterSet::numBehaviorPatterns; k++) {
                //additional costs are per planning period, given 6 sprays
                double additionalCosts = agents[i][j].behaviorCosts[k] * 6 / 4;
                double EV = behaviorPatterns[k]->getExpectedValue(agents[i][j], risk, totalProjectingDays, 
                                                                period_t, ParameterSet::planningLength,
                                                                agents[i][j].getSprayEfficacy(), agents[i][j].getAlpha(),
                                                                additionalCosts);
                if ( (EV > maxExpectedValue) || k == 0 ) {
                    maxExpectedValue = EV;
                    maxEVIndex = k;
                }
                switch (k) {
                    case 0:
                        agents[i][j].lastNAEV = EV;
                        break;
                    case 1:
                        agents[i][j].lastISEV = EV;
                        break;
                    case 2:
                        agents[i][j].lastGSEV = EV;
                        break;
                    default:
                        break;
                }
            }
            if (agents[i][j].getBehavior() != behaviorPatterns[maxEVIndex]) {
                planBehavior(&agents[i][j], behaviorPatterns[maxEVIndex]);
                agents[i][j].setBehavior(behaviorPatterns[maxEVIndex]);
       
            }
        }
    }*/
    return;

}

/********************************************************
* Phase 4
* Plan Actions
*********************************************************/
void Phase4() {
    //only plan manually during the new year
    int period_t = bioABM::getModelDay();
    if ( (period_t % 365) != 0 ) { return; }
    for (int i = 0; i < ParameterSet::gridLength; i++) {
        for(int j=0; j<ParameterSet::gridWidth;j++){
            for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                agents[i][j].behaviorPatterns[k]->PlanActions();
            }
        }
    }
    //DEPRECATED
    /*
    for (int i = 0; i < ParameterSet::gridLength; i++) {
        for (int j = 0; j < ParameterSet::gridWidth; j++) {
            for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                agents[i][j].behaviorPatterns[k]->PlanActions(&agents[i][j]);
            }
        }
    }*/
}

/***********************************************
* Phase 5: Accounting
* Economic accounting for crops and mitigation
***********************************************/
void Phase5() {
    int t = bioABM::getModelDay();
    int rel_t = t % 365;
    
    for (int i = 0; i < ParameterSet::gridLength; i++) {
        for(int j=0; j<ParameterSet::gridWidth;j++){
        int* ibounds = agents[i][j].getIBounds();
        int* jbounds = agents[i][j].getJBounds();
        int numCrops = (ibounds[1] - ibounds[0]) * (jbounds[1] - jbounds[0]);

        if (rel_t == 0) {
            //VC
            //agents[i][j].costs += numCrops * agents[i][j].getCrop()->getVariableCost();
            //FC
            //agents[i][j].costs += agents[i][j].getFixedCosts();
            agents[i][j].costs += agents[i][j].getCrop()->costs;
        }
       
        //Harvest
        if (agents[i][j].getCrop()->isHarvestPeriod(rel_t)) {
            
            for (int k = ibounds[0]; k < ibounds[1]; k++) {
                for (int l = jbounds[0]; l < jbounds[1]; l++) {
                    //No yield if tree is dead
                    if (!bioABM::isTreeAlive(k,l)) {
                        continue;
                    }
                     
                    //Projected severity based on days since initial infection
                    double severity = bioABM::getSeverityAt(k, l);
                    //Yield of crop at projected age
                    double returns = agents[i][j].getCrop()->getReturns();
                    //Infected yield: Units yielded times projected decay
                    double adjustedReturns = returns * getInfectedYield(severity);
                    agents[i][j].returns += adjustedReturns;

                }
            }
        }

        // Storing the information in the grovers bank and calculating the memory length

        double meanSeverity = getMeanHLB(agents[i][j]);

        /*
        This is used when we have multiple behaviour type
        
        stringstream strategyNames;
        for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                    strategyNames << agents[i][j].behaviorPatterns[k]->getName();
                    if (k != agents[i][j].behaviorPatterns.size() - 1) {
                        strategyNames << "-";
                    }
        }
        agentsinfo[i][j].setgroverbankparameters((agents[i][j].returns - agents[i][j].costs),meanSeverity,strategyNames.str());*/


        agentsinfo[i][j].setgroverbankparameters((agents[i][j].returns - agents[i][j].costs),meanSeverity, agents[i][j].behaviorPatterns[0]->getName());

         if( (bioABM::getModelDay()%365) == 0)
           {
                int year = bioABM::getModelDay()/365;
                memorylengthyear[year-1] = (agents[i][j].returns - agents[i][j].costs);
                double valuefunction = 0;
                if(year < memorylength)
                {
                   valuefunction = ((memorylength-(bioABM::getModelDay()/365)) * memoryvalue) + (agents[i][j].returns - agents[i][j].costs);
                }
                else if(year == memorylength) 
                {
                    valuefunction = (agents[i][j].returns - agents[i][j].costs);
                }
                else
                {
                    valuefunction = memorylengthyear[year-1] - memorylengthyear[year-memorylength-1];
                }

                //cout <<year-1<<"~~"<<year<<"~~"<< memorylengthyear[year-1] << "~~" << valuefunction << "~~" << (agents[i][j].returns - agents[i][j].costs) << endl;
                
                agents[i][j].setValuefunction(valuefunction);
           }

       
        //DEPRECATED: Responsibility for strategy accounting moved to strategy objects
        //Mitigation costs 
        /*
        if (rel_t % ParameterSet::planningLength == 0) {
            for (int k = 0; k < agents[i].behaviorPatterns.size(); k++) {
                agents[i].costs += agents[i].behaviorPatterns[k]->getVariableCosts();
            }
        }*/
         }
    }
    // DEPRECATED
    
   /* for (int i = 0; i < ParameterSet::gridLength; i++) {
        for (int j = 0; j < ParameterSet::gridWidth; j++) {
            int* ibounds = agents[i][j].getIBounds();
            int* jbounds = agents[i][j].getJBounds();
            int numCrops = (ibounds[1] - ibounds[0]) * (jbounds[1] - jbounds[0]);

            if (rel_t == 0) {
                //VC
                //agents[i][j].costs += numCrops * agents[i][j].getCrop()->getVariableCost();
                //FC
                //agents[i][j].costs += agents[i][j].getFixedCosts();
                agents[i][j].costs += agents[i][j].getCrop()->costs;
            }

            //Harvest
            if (agents[i][j].getCrop()->isHarvestPeriod(rel_t)) {
                for (int k = ibounds[0]; k < ibounds[1]; k++) {
                    for (int l = jbounds[0]; l < jbounds[1]; l++) {
                        //No yield if tree is dead
                        if (!bioABM::isTreeAlive(k,l)) {
                            continue;
                        }
                        //Projected severity based on days since initial infection
                        double severity = bioABM::getSeverityAt(k, l);
                        //Yield of crop at projected age
                        double returns = agents[i][j].getCrop()->getReturns();
                        //Infected yield: Units yielded times projected decay
                        double adjustedReturns = returns * getInfectedYield(severity);
                        agents[i][j].returns += adjustedReturns;

                    }
                }
            }

            //Mitigation costs
            if (rel_t % ParameterSet::planningLength == 0) {
                for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                    agents[i][j].costs += agents[i][j].behaviorPatterns[k]->getVariableCosts();
                }
            }
        }
    }*/
    return;
}

/*************************************************************
* Write CSV Line
***************************************************************/
void writeCSVLine() {
    for (int i = 0; i < ParameterSet::gridLength ; i++) {
        for(int j=0; j<ParameterSet::gridWidth;j++){
            double meanSeverity = getMeanHLB(agents[i][j]);
            stringstream strategyNames;
            stringstream strategyParams;
            if (agents[i][j].behaviorPatterns.empty()) {
                strategyNames << "NoAction";
                strategyParams << "NA";
            }
            else {
                for (int k = 0; k < agents[i][j].behaviorPatterns.size(); k++) {
                    strategyNames << agents[i][j].behaviorPatterns[k]->getName();
                    strategyParams << agents[i][j].behaviorPatterns[k]->getParams();
                    if (k != agents[i][j].behaviorPatterns.size() - 1) {
                        strategyNames << "-";
                        strategyParams << "-";
                    }
                }
            }
            outputFile << bioABM::getModelDay() << ","; 
            outputFile << "g" << i << j << ",";
            outputFile << agents[i][j].costs << ",";
            outputFile << agents[i][j].returns << ",";
            outputFile << (agents[i][j].returns - agents[i][j].costs) << ",";
            outputFile << meanSeverity << ",";
            outputFile << strategyNames.str() << ",";
            outputFile << strategyParams.str() << ",";
            outputFile << experimentID << ","; 
            outputFile << getDeadTrees(agents[i][j]) << endl;   

            }
    }
}
/*************************************************************
* Run Model
* Runs through the phases of the model until the simulation is
* complete
**************************************************************/
void runModel() {
    while (bioABM::getModelDay() <= bioABM::getModelDuration()) {
        // Stage 1: Psyllid Growth and Movement
        Phase1();
             
        //cout << "Period " << bioABM::getModelDay() << endl;
        if (!ParameterSet::biologicalRun) {
            if (bioABM::getModelDay() % ParameterSet::planningLength == 0) {
              //cout << "Planning period!\n";
            }
            // Stage 2: Execution of Planned Actions
            Phase2();
             
            // Stage 3: Behavior determination
            //Phase3();

            //Stage 4: Planning
            Phase4();
            

            //Stage 5: Accounting
            Phase5();
            writeCSVLine();
            
        }
    }
    outputFile.close();
    bioABM::finishRun();
}

/******************************************************
* Parse Parameter File
* Parses a parameter file 
* ****************************************************/
void parseParameterFile(string fileName) {
    try {
        ifstream is(fileName);
        cereal::JSONInputArchive archive(is);
        archive(ParameterSet::planningLength, ParameterSet::freshYield, ParameterSet::juiceYield,
            ParameterSet::freshPrice, ParameterSet::juicePrice, ParameterSet::costs, ParameterSet::biologicalRun,
            ParameterSet::projectionLength, outputFilename, harvestDays, strategyParameters, strategyFlags, agencyFlags, experimentID,agentsbankinfo,
           memoryvalue);
    }
    catch (exception e) {
        cout << "ERROR WITH ECON JSON:" << e.what() << endl;
        exit(-1);
    }
}   

Commodity getCommodity() {
    //get harvests
    vector<string> sHarvests = split(harvestDays, ",");
    vector<int> harvests;
    for (string s: sHarvests) {
        harvests.push_back(stoi(s));
    }
    Commodity retVal(ParameterSet::freshYield, ParameterSet::juiceYield, ParameterSet::freshPrice, ParameterSet::juicePrice, ParameterSet::costs, harvests);
    return retVal;
}




/*****************************************************
* Main
*****************************************************/
int main(int argc, char ** argv) {
    string econConfigFile;
    string bioConfigFile;
    if (argc == 3) {
        econConfigFile = argv[1];
        bioConfigFile = argv[2];
        
    } 
    else {
        cout << "Using default filenames\n";
        econConfigFile = "configs/econConfig.json";
        bioConfigFile = "configs/bioConfig.json";
    }
    parseParameterFile(econConfigFile);
    bioABM::parseParameterFile(bioConfigFile);


    // Set up the behavior patterns, parameters arent used
    //IndividualAction individualSpray = IndividualAction(60, ParameterSet::sprayCost, ParameterSet::planningLength);
    
    //DEPRECATED: No pre-set behavior list for the time being

    /*
    GroupAction groupSpray = GroupAction(21, ParameterSet::sprayCost + 0, ParameterSet::planningLength,
                                        bioABM::getFallStart(), bioABM::getSpringStart(), bioABM::getSummerStart());
    NoAction noaction = NoAction();

    RogueTrees rougeing = RogueTrees(0, ParameterSet::rogueFrequency, ParameterSet::rogueRadius);

    //List of possible behaviors
    behaviorPatterns[0] = &noaction;
    behaviorPatterns[1] = &groupSpray;
    behaviorPatterns[2] = &rougeing;
    */
    bioABM::setExperimentID(experimentID);
    outputFile.open(outputFilename);
    outputFile
        << fixed << "t,id,costs,returns,profit,hlb_severity,strategy_names, strategy_params, experiment_id" << endl;

    InitialiseCHMA(getCommodity());
   
    runModel();
    

    return 0;
}
