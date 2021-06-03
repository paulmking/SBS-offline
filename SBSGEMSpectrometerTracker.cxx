#include <iostream>
#include <string>

#include "THaTrackingDetector.h"
#include "THaRunBase.h"
#include "THaCrateMap.h"
#include "THaAnalysisObject.h"

#include "SBSGEMSpectrometerTracker.h"
#include "SBSGEMModule.h"

SBSGEMSpectrometerTracker::SBSGEMSpectrometerTracker( const char* name, const char* desc, THaApparatus* app ):
  THaTrackingDetector(name,desc,app) : SBS{

        fPlanes.clear();
	fIsMC = false;//by default!
        fCrateMap = 0;	
}

SBSGEMSpectrometerTracker::~SBSGEMSpectrometerTracker(){
    return;
}


THaAnalysisObject::EStatus SBSGEMSpectrometerTracker::Init( const TDatime& date ){
    assert( fCrateMap == 0 );
 
    // Why THaTrackingDetector::Init() here? THaTrackingDetector doesn't implement its own Init() method
    THaAnalysisObject::EStatus status = THaTrackingDetector::Init(date);

    if( status == kOK ){
        for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
            status = (*it)->Init(date);
            if( status != kOK ){
                return status;
            }
        }
    } else {
        return kInitError;
    }

    return kOK;
}


Int_t SBSGEMSpectrometerTracker::ReadDatabase( const TDatime& date ){
    std::cout << "[Reading SBSGEMSpectrometerTracker database]" << std::endl;

    fIsInit = kFALSE;

    FILE* file = OpenFile( date );
    if( !file ) return kFileError;

    //As far as I can tell, this doesn't do anything yet (AJRP):
    Int_t err = ReadGeometry( file, date );
    if( err ) {
        fclose(file);
        return err;
    }

    std::string planeconfig;
    std::vector<Int_t> *cmap = new std::vector<Int_t>;
    //it appears that cmap is not actually used yet in any way. TBD

    DBRequest request[] = {
        { "planeconfig",       &planeconfig,       kString   },
        { "cratemap",          cmap,               kIntV     },
        { "is_mc",             &fIsMC,             kInt, 0, 1},
        {0}
    };

    Int_t status = kInitError;
    err = LoadDB( file, date, request, fPrefix );
    fclose(file);

    //vsplit is a Podd function that "tokenizes" a string into a vector<string> by whitespace:
    std::vector<std::string> planes = vsplit(planeconfig);
    if( planes.empty()) {
            Error("", "[SBSGEMSpectrometerTracker::ReadDatabase] No planes defined");
    }

    for (std::vector<std::string>::iterator it = planes.begin() ; it != planes.end(); ++it){
      fPlanes.push_back(new SBSGEMPlane( (*it).c_str(), (*it).c_str(), this, fIsMC));
    }

    status = kOK;

    if( status != kOK )
        return status;

    fIsInit = kTRUE;
    
    return kOK;
}


Int_t SBSGEMSpectrometerTracker::Begin( THaRunBase* run ){
    for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
        (*it)->Begin(run);
    }

    return 0;
}

void SBSGEMSpectrometerTracker::Clear( Option_t *opt ){
    for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
        (*it)->Clear(opt);
    }

    return;
}

Int_t SBSGEMSpectrometerTracker::Decode(const THaEvData& evdata ){
  //return 0;
  //std::cout << "[SBSGEMSpectrometerTracker::Decode]" << std::endl;

    for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
      (*it)->Decode(evdata);
    }

    return 0;
}


Int_t SBSGEMSpectrometerTracker::End( THaRunBase* run ){
    for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
        (*it)->End(run);
    }


    return 0;
}

void SBSGEMSpectrometerTracker::Print(const Option_t* opt) const {
    std::cout << "GEM Stand " << fName << " with " << fPlanes.size() << " planes defined:" << std::endl;
    /*
    for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
        std::cout << "\t"
        (*it)->Print(opt);
    }
    */
    for( unsigned int i = 0; i < fPlanes.size(); i++ ){
        fPlanes[i]->Print(opt);
    }

    return;
 }


void SBSGEMSpectrometerTracker::SetDebug( Int_t level ){
      THaTrackingDetector::SetDebug( level );
    for (std::vector<SBSGEMPlane *>::iterator it = fPlanes.begin() ; it != fPlanes.end(); ++it){
        (*it)->SetDebug(level);
    }

    return;
}

Int_t SBSGEMSpectrometerTracker::DefineVariables( EMode mode ){
    if( mode == kDefine and fIsSetup ) return kOK;
    fIsSetup = ( mode == kDefine );
    RVarDef vars[] = {
//        { "trkstat", "Track reconstruction status",  "fTrkStat" },
        { 0 },
    };
    DefineVarsFromList( vars, mode );

    return 0;
}


Int_t SBSGEMSpectrometerTracker::CoarseTrack( TClonesArray& tracks ){
    return 0;
}
Int_t SBSGEMSpectrometerTracker::FineTrack( TClonesArray& tracks ){
    return 0;
}
