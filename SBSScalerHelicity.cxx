//*-- Author :    Paul King, August 2021
////////////////////////////////////////////////////////////////////////
//
// SBSScalerHelicity
//
// Based on the Podd class HallA/THaQWEAKHelicity
//
// Helicity of the beam from QWEAK electronics in delayed mode
//    +1 = plus,  -1 = minus,  0 = unknown
//
// Also supports in-time mode with delay = 0
// 
////////////////////////////////////////////////////////////////////////

#include "SBSScalerHelicity.h"
#include "THaEvData.h"
#include "TH1F.h"
#include "TMath.h"
#include <iostream>

using namespace std;

//_____________________________________________________________________________
SBSScalerHelicity::SBSScalerHelicity( const char* name, const char* description,
				    THaApparatus* app ):
  THaHelicityDet( name, description, app ), 
  fOffsetTIRvsRing(3), fQWEAKDelay(8), fMAXBIT(30), 
  fQWEAKNPattern(4), HWPIN(true), fQrt(1), fTSettle(0),fValidHel(false),
  fHelicityLastTIR(0),fPatternLastTIR(0), fErrorCode(0), fRing_NSeed(0),
  fRingU3plus(0),fRingU3minus(0),
  fRingT3plus(0),fRingT3minus(0),
  fRingT5plus(0),fRingT5minus(0),
  fRingT10plus(0),fRingT10minus(0),
  fRingTimeplus(0), fRingTimeminus(0),
  fRingSeed_reported(0),fRingSeed_actual(0),
  fRingPhase_reported(0),fRing_reported_polarity(0),
  fRing_actual_polarity(0), fEvtype(-1), fHisto(NHIST, nullptr)
{
  for (UInt_t j=0; j<32; j++){
    fScalerCumulative[j] = 0;
  }
}

//_____________________________________________________________________________
SBSScalerHelicity::SBSScalerHelicity()
  : fOffsetTIRvsRing(3), fQWEAKDelay(8), fMAXBIT(30),
    fQWEAKNPattern(4), HWPIN(true), fQrt(1), fTSettle(0),fValidHel(false),
    fHelicityLastTIR(0),fPatternLastTIR(0), fErrorCode(0), fRing_NSeed(0),
    fRingU3plus(0),fRingU3minus(0),
    fRingT3plus(0),fRingT3minus(0),
    fRingT5plus(0),fRingT5minus(0),
    fRingT10plus(0),fRingT10minus(0),
    fRingTimeplus(0), fRingTimeminus(0),
    fRingSeed_reported(0),fRingSeed_actual(0),
    fRingPhase_reported(0),fRing_reported_polarity(0),
    fRing_actual_polarity(0), fEvtype(-1), fHisto(NHIST, nullptr)
{
  // Default constructor for ROOT I/O
  for (UInt_t j=0; j<32; j++){
    fScalerCumulative[j] = 0;
  }
}

//_____________________________________________________________________________
SBSScalerHelicity::~SBSScalerHelicity() 
{
  RemoveVariables();

  // for( Int_t i = 0; i < NHIST; ++i ) {
  //   delete fHisto[i];
  // }
}

//_____________________________________________________________________________
Int_t SBSScalerHelicity::DefineVariables( EMode mode )
{
  // Initialize global variables

  //  cout << "Called SBSScalerHelicity::DefineVariables with mode == "
  //       << mode << endl;

  // Define standard variables from base class
  Int_t ret = THaHelicityDet::DefineVariables( mode );
  if( ret )
    return ret;

  const RVarDef var[] = {
    { "qrt",      "actual qrt for TIR event",        "fQrt" },
    { "hel",      "actual helicity for TIR event",   "fHelicity" },
    { "tsettle",  "TSettle for TIR event",           "fTSettle"},
    { "U3plus",   "U3 plus",                         "fRingU3plus"},
    { "U3minus",  "U3 minus",                        "fRingU3minus"},
    { "T3plus",   "T3 plus",                         "fRingT3plus"},
    { "T3minus",  "T3 minus",                        "fRingT3minus"},
    { "T5plus",   "T5 plus",                         "fRingT5plus"},
    { "T5minus",  "T5 minus",                        "fRingT5minus"},
    { "T10plus",  "T10 plus",                        "fRingT10plus"},
    { "T10minus", "T10 minus",                       "fRingT10minus"},
    { "Timeminus","Time minus",                      "fRingTimeminus"},
    { "Timeplus", "Time plus",                       "fRingTimeplus"},
    { nullptr }
  };
  //  cout << "now actually defining stuff, prefix = " << fPrefix << endl;
  return DefineVarsFromList( var, mode );
}
//_____________________________________________________________________________

void SBSScalerHelicity::PrintEvent( UInt_t evtnum )
{

  cout<<" ++++++ SBSScalerHelicity::Print ++++++\n";
  cout << dec << "--> Data for spectrometer " << GetName() << endl;
  cout << "  evtype " << fEvtype<<endl;
  cout << " event number ="<<evtnum<<endl;
  cout << " == Input register data =="<<endl;
  cout << " helicity="<<fHelicityTir<<" Pattern ="<<fPatternTir
       <<" TSettle="<<fTSettleTir<<" TimeStamp="<<fTimeStampTir<<endl;
  cout << " == Ring data =="<<endl;
  UInt_t sumu3=0;
  UInt_t sumt3=0;
  UInt_t sumt5=0;
  UInt_t sumt10=0;
  UInt_t sumtime=0;
    
  for (UInt_t i=0;i<fIRing;i++)
    {
      cout<<" iring="<< i<<" helicity="<<fHelicityRing[i]
	  <<" Pattern="<<fPatternRing[i]<<" timestamp="<<fTimeStampRing[i]
	  <<" T3="<<fT3Ring[i]<<" T5="<<fT5Ring[i]<<" T10="<<fT10Ring[i]
	  <<" U3="<<fU3Ring[i]
	  <<endl;
      sumu3+=fU3Ring[i];
      sumt3+=fT3Ring[i];
      sumt5+=fT5Ring[i];
      sumt10+=fT10Ring[i];
      sumtime+=fTimeStampRing[i];
    }
    
  cout<<" == outputs ==\n";
  cout<<" fQrt="<<fQrt<<" Helicity="<<fHelicity<<" TSettle="<<fTSettle<<endl;
  cout<<" U3  plus="<<fRingU3plus<<"  minus="<<fRingU3minus<<" sum u3="<<sumu3<<endl;
  cout<<" T3  plus="<<fRingT3plus<<"  minus="<<fRingT3minus<<" sum t3="<<sumt3<<endl;
  cout<<" T5  plus="<<fRingT5plus<<"  minus="<<fRingT5minus<<" sum t5="<<sumt5<<endl;
  cout<<" T10 plus="<<fRingT10plus<<"  minus="<<fRingT10minus<<" sum t10="<<sumt10<<endl;
  cout<<" time plus="<<fRingTimeplus<<" minus="<<fRingTimeminus<<" sum time="<<sumtime<<endl;
  cout<<" +++++++++++++++++++++++++++++++++++++\n";
}

//_____________________________________________________________________________
Int_t SBSScalerHelicity::ReadDatabase( const TDatime& date )
{
  // Read general HelicityDet database values (e.g. fSign)
  Int_t st = THaHelicityDet::ReadDatabase( date );
  if( st != kOK )
    return st;

  // Read QWEAK readout parameters (ROC addresses etc.)
  st = SBSScalerHelicityReader::ReadDatabase( GetDBFileName(), GetPrefix(),
					  date, fQWEAKDebug );
  if( st != kOK )
    return st;


  // for now bypass reading the inputs from the database;
  fMAXBIT=30;
  fOffsetTIRvsRing=3;
  fQWEAKDelay=8;
  // maximum of event in the pattern, for now we are working with quartets
  // careful, the first value here should always +1
  fPatternSequence = {1,-1,-1,1};
  HWPIN=true;

  return kOK;
}

//_____________________________________________________________________________
Int_t SBSScalerHelicity::Begin( THaRunBase* )
{
  SBSScalerHelicityReader::Begin();

  fHisto[0] = new TH1F("hel.seed","hel.seed",32,-1.5,30.5);
  fHisto[1] = new TH1F("hel.error.code","hel.error.code",35,-1.5,33.5);
 
  return 0;
}

//_____________________________________________________________________________
void SBSScalerHelicity::FillHisto()
{
  fHisto[0]->Fill(fRing_NSeed);
  fHisto[1]->Fill(fErrorCode);
}
//_____________________________________________________________________________
void SBSScalerHelicity::SetErrorCode(Int_t error)
{
 // used as a control for the helciity computation
  // 2^0: if the reported number of events in a  pattern is larger than fQWEAKNPattern
  // 2^1: if the offset between the ring reported value and TIR value is not fOffsetTIRvsRing
  // 2^2: if the reported time in the ring is 0
  // 2^3: if the predicted reported helicity doesn't match the reported helicity in the ring
  // 2^4: if the helicity cannot be computed using the SetHelicity routine
  // 2^5: if seed is being gathered

  if(fErrorCode==0)
    fErrorCode=(1<<error);
  // only one reported error at the time
}

//_____________________________________________________________________________
void SBSScalerHelicity::Clear( Option_t* opt ) {
  // Clear event-by-event data

  THaHelicityDet::Clear(opt);
  SBSScalerHelicityReader::Clear(opt);
  fEvtype = 0;
  fHelicity = kUnknown;

  fQrt = 0;
  fTSettle = 0;
  fRingU3plus = 0;
  fRingU3minus = 0;
  fRingT3plus = 0;
  fRingT3minus = 0;
  fRingT5plus = 0;
  fRingT5minus = 0;
  fRingT10plus = 0;
  fRingT10minus = 0;
  fRingTimeplus = 0;
  fRingTimeminus = 0;
  fErrorCode = 0;
}

//_____________________________________________________________________________
Int_t SBSScalerHelicity::Decode( const THaEvData& evdata )
{

  // Decode Helicity data.
  // Return 1 if helicity was assigned, 0 if not, <0 if error.

  /*
   *  std::cout << "\n\nCumulative counts:   chan0, chan9, chan15:  " << std::dec
   *	    << fScalerCumulative[0] << " " << fScalerCumulative[9] << " "
   *	    << fScalerCumulative[15]
   *	    << std::endl;
   */
  
  Int_t err = ReadData( evdata ); // from SBSScalerHelicityReader class
  if( err ) {
    Error( Here("SBSScalerHelicity::Decode"), "Error decoding helicity data." );
    return err;
  }
  //Here should accumulate the fScalerCumulative values, if we've read new entries.
  if (fIRing>0){
    for (UInt_t i=0; i<fIRing; i++){
      for (UInt_t j=0; j<32; j++){
	fScalerCumulative[j] += fScalerRing[i][j];
      }
    }
    std::cout << "Cumulative counts:   chan0, chan9, chan15:  " << std::dec
	      << fScalerCumulative[0] << " " << fScalerCumulative[9] << " "
	      << fScalerCumulative[15]
	      << std::endl;
  }

     
  fEvtype = evdata.GetEvType();
  /*
   *  TODO:  the follow funcitons were for the "old" data structure, and should
   *         be reconsidered if they should be adapted or removed.
  LoadHelicity(evdata.GetEvNum()); 
  if(fQWEAKDebug>1)
    PrintEvent(evdata.GetEvNum());
  CheckTIRvsRing(evdata.GetEvNum());
  if(fErrorCode==0)  
    fValidHel=true;
  else
    fValidHel=false;
  */
  FillHisto();

  return 0;
}
//_____________________________________________________________________________
void SBSScalerHelicity::CheckTIRvsRing( UInt_t eventnumber )
{
  // here one checks that the offset between the TIR helicity reports 
  // and the Ring report is as expected (fQWEAKOffset)
  // This is a simplified comparison: ie not every  TIR readings is 
  // compared to the Ring readings for example, and offset=3
  // for simplicity the comparison only if the  ring buffer contains at least 
  // fQWEAKOffset readings

  static const char* const here = "SBSScalerHelicity::CheckTIRvsRing";

  if(fIRing>=fOffsetTIRvsRing)
    {
      // compare the two values (last TIR) and reading in the current ring
      if(fHelicityRing[fOffsetTIRvsRing-1]!=fHelicityLastTIR
	 || fPatternRing[fOffsetTIRvsRing-1]!=fPatternLastTIR) 
	{
	  fHelicity=kUnknown;
	  fRing_NSeed=0;
	  if(fQWEAKDebug>0)
	    {
	      cout<<here<<" BAD !! the offset between the helicity ring and the input register ";
	      cout << "is not what is expected: reset the seed !! event#" << eventnumber << "\n";
	    }
	  SetErrorCode(1);

	  if(fQWEAKDebug>1)
	    {
	      cout<<"=====================================\n";
	      cout<<here<<endl;
	      cout << " Event number =" << eventnumber << endl;
	      cout<<" fOffsetTIRvsRing ="<<fOffsetTIRvsRing;
	      cout<<" fHelicityLastTIR ="<<fHelicityLastTIR;
	      cout<<" fPatternLastTIR ="<<fPatternLastTIR;
	      cout<<" fIRing="<<fIRing<<endl;
	      cout<<"RING data: helicity="<<fHelicityRing[fOffsetTIRvsRing-1]
		  <<" pattern="<<fPatternRing[fOffsetTIRvsRing-1]<<endl;
	    }
	} 
    }
  fHelicityLastTIR= fHelicityTir;
  fPatternLastTIR=fPatternTir;
}

//_____________________________________________________________________________
Int_t SBSScalerHelicity::End( THaRunBase* )
{
  // End of run processing. Write histograms.
  SBSScalerHelicityReader::End();

  for( Int_t i = 0; i < NHIST; ++i )
    fHisto[i]->Write();

  return 0;
}

//_____________________________________________________________________________
void SBSScalerHelicity::SetDebug( Int_t level )
{
  // Set debug level of this detector as well as the SBSScalerHelicityReader 
  // helper class.

  THaHelicityDet::SetDebug( level );
  fQWEAKDebug = level;
}

//_____________________________________________________________________________
void SBSScalerHelicity::LoadHelicity( UInt_t eventnumber )
{
  static const char* const here = "SBSScalerHelicity::LoadHelicity";

  for (UInt_t i=0;i<fIRing;i++)
    { 
      //Check for the number of events between two Pattern signals
      if(fPatternRing[i]==1)
	{
	  // could  add a check that the number of pattern is what is expected. EG 4 for a quartet
	  fRingPhase_reported=0;
	}
      else
	{
	  fRingPhase_reported+=1;
	}
      if(fRingPhase_reported>fQWEAKNPattern)
	{
	  if(fQWEAKDebug>0)
            cout << here << " Reset seed: The pattern has too many elements !! "
                 << "Should only go up to " << fQWEAKNPattern
                 << " but is now " << fRingPhase_reported
                 << " at event #=" << eventnumber
                 << endl;
	  fRing_NSeed=0;
	  SetErrorCode(0);
	}
      
      //Check that events in the ring is not empty, using the timestamp
      if(fTimeStampRing[i]==0)
	{
	  fRing_NSeed=0;
	  SetErrorCode(2);
	}

      // if seed has been gathered: 
      // check event by event that the seed make sense:
      // fRing_polarity!=fHelcityRing[i]
      // this should come before the section for the seed gathering::
      // do not change this order
      if(fRing_NSeed==fMAXBIT && fPatternRing[i]==1)
	{
	  fRing_reported_polarity=RanBit30(fRingSeed_reported);	      
	  fRing_actual_polarity=RanBit30(fRingSeed_actual);
	  if(fRing_reported_polarity!=fHelicityRing[i])
	    {
	      if(fQWEAKDebug>0)
		cout<<here<<" Catastrophe  !!"
		    <<" predicted helicity doesn't match reported helicity !!!"
		    <<" event #="<<eventnumber
		    <<endl;
	      if(fQWEAKDebug>1)
                cout<<" iring="<<Form("%02d", i)
                    <<" predicted helicity="<<fRing_reported_polarity
                    <<" fHelicityRing["<<i<<"]="<<fHelicityRing[i]<<endl;
	      fRing_NSeed=0;
	      SetErrorCode(3);
	    }
	}

      // Here is the seed gathering if necessary
      if(fRing_NSeed<fMAXBIT && fPatternRing[i]==1)
	{
	  SetErrorCode(5);
	  fRingSeed_reported=
	    ((fRingSeed_reported<<1)&0x3FFFFFFF)|fHelicityRing[i];
	  fRing_NSeed+=1;
	  if (fRing_NSeed==fMAXBIT)
	    {
	      fRingSeed_actual=fRingSeed_reported;
	      UInt_t advance=0;
	      //take the delay into account
	      for(UInt_t j=0;j<fQWEAKDelay;j++)
		{	      
		  advance+=1;
		  if( advance == fQWEAKNPattern)
		    {
		      fRing_actual_polarity=RanBit30(fRingSeed_actual);
		      advance=0;
		    }
		}
	    }
	}

      //now compute helicity related quantities
      EHelicity localhelicity = kUnknown;
      if(fRing_NSeed==fMAXBIT)
	{
	  localhelicity=SetHelicity(fRing_actual_polarity,fRingPhase_reported); 	  
	  if(localhelicity==kPlus)
	    {
	      fRingU3plus+=fU3Ring[i];
	      fRingT3plus+=fT3Ring[i];
	      fRingT5plus+=fT5Ring[i];
	      fRingT10plus+=fT10Ring[i];
	      fRingTimeplus+=fTimeStampRing[i];
	    }
	  else if(localhelicity==kMinus)
	    {
	      fRingU3minus+=fU3Ring[i];
	      fRingT3minus+=fT3Ring[i];
	      fRingT5minus+=fT5Ring[i];
	      fRingT10minus+=fT10Ring[i];
	      fRingTimeminus+=fTimeStampRing[i];
	    }
	  else 
	    {
	      if(fQWEAKDebug>0)		
		cout<<here<<" TROUBLE !! Local helicity doesn't make sense"
		    <<" event #="<<eventnumber<<endl;
	      fRing_NSeed=0;
	      SetErrorCode(4);
	    }
	  
	}

      if(fQWEAKDebug>1)		
	{
	  cout<<" iring="<<Form("%02d", i)
 	      <<" hel,pat="
 	      <<fHelicityRing[i]<<" ,"
 	      <<fPatternRing[i]
	      <<" phase="<<fRingPhase_reported
	      <<" NSeed="<<fRing_NSeed
	      <<" Ring(pol reported="<<fRing_reported_polarity
	      <<", actual="<<fRing_actual_polarity
	      <<", actual hel="<<localhelicity
	      <<")"
	      <<endl;

	}      
    }


  // now go and get the true helicity for the event in the TIR 
  if(fRing_NSeed==fMAXBIT)
    {
      if(fTSettleTir==1)
	{
	  fHelicity=kUnknown;
	  fTSettle=1;
	}
      else
	{
	  fTSettle=0;
	  UInt_t localfPhase=fRingPhase_reported;
	  UInt_t localfSeed=fRingSeed_actual;
	  UInt_t localfPolarity=fRing_actual_polarity;
	  
	  for(UInt_t i=0; i<fOffsetTIRvsRing;i++)
	    {
	      localfPhase+=1;
	      if( localfPhase == fQWEAKNPattern)
		{
		  localfPhase=0;
		  localfPolarity=RanBit30(localfSeed);
		}
	    }
	  fHelicity=SetHelicity(localfPolarity,localfPhase);
	  if(fPatternTir==1)
	    fQrt=1;
	  else
	    fQrt=0;
	  if(fHelicity==kUnknown)
	    {
	      if(fQWEAKDebug>0)
		std::cout<<"TROUBLE !!! when predicting the actual helicity for the CODA "
			 <<" event #="<<eventnumber<<endl;
	      fRing_NSeed=0;
	    } 	  
	}
    }
  else
    {
      fHelicity=kUnknown;
    }
}

//_____________________________________________________________________________
THaHelicityDet::EHelicity
SBSScalerHelicity::SetHelicity( UInt_t polarity, UInt_t phase )
{
  // here predicted_reported_helicity can have a value of 0 or 1
  // fPatternSequence[fRingPhase_reported] can have a value of 1 or -1

  THaHelicityDet::EHelicity localhel = kUnknown;

  Int_t select = static_cast<Int_t>(polarity) + fPatternSequence[phase];
  if( select == -1 || select == 2 ) {
    if( HWPIN )
      localhel = kPlus;
    else
      localhel = kMinus;
  } else if( select == 1 || select == 0 ) {
    if( HWPIN )
      localhel = kMinus;
    else
      localhel = kPlus;
  }

  // std::cout<<" ++++ SBSScalerHelicity::SetHelicity \n";
  // std::cout<<" actual ring  polarity="<<polarity
  // 	   <<" fRingPhase_reported="<<fPatternSequence[phase]
  // 	   <<" HWP="<<HWPIN
  // 	   <<" select="<<select
  // 	   <<" Helicity="<<localhel<<endl;
  
  return localhel;
}

//_____________________________________________________________________________
UInt_t SBSScalerHelicity::RanBit30( UInt_t& ranseed )
{

  bool bit7    = (ranseed & 0x00000040) != 0;
  bool bit28   = (ranseed & 0x08000000) != 0;
  bool bit29   = (ranseed & 0x10000000) != 0;
  bool bit30   = (ranseed & 0x20000000) != 0;

  UInt_t newbit = (bit30 ^ bit29 ^ bit28 ^ bit7) & 0x1;

  if(ranseed<=0) {
    if(fQWEAKDebug>1)
      std::cerr<<"ranseed must be greater than zero!"<<"\n";

    newbit = 0;
  }
  ranseed =  ( (ranseed<<1) | newbit ) & 0x3FFFFFFF;
  //here ranseed is changed
  if( fQWEAKDebug > 1 ) {
    cout << "SBSScalerHelicity::RanBit30, newbit=" << newbit << "\n";
  }

  // Returns 0 or 1
  return newbit;
}

ClassImp(SBSScalerHelicity)

