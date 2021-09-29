/**************************************************************************************************/
/*  Purpose:    This utility, suntimes, calculates sunrise and sunset times, also twilights.      */
/*  Author:     Copyright (c) 2021, W.B.Hill <mail@wbh.org> All rights reserved.                  */
/*  License:    Choose either BSD 2 clause or GPLv2 - see included file LICENSE.                  */
/*  BasedOn:    https://www.edwilliams.org/sunrise_sunset_example.htm                             */
/*  Notes:      Requires libconfuse, https://github.com/libconfuse/libconfuse                     */
/**************************************************************************************************/


// Standard infrastructure
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
// Scarey stuff.
#include <math.h>
// Time functions.
#include <time.h>
// For basename()
#include <libgen.h>
// Options parsing.
#include <getopt.h>
// Required by strtol()
#include <limits.h>

// Config file reading.
#include <confuse.h>

// Compile-time defults, globals.
#include "suntimes.h"


// Convert macros to strings.
#define STR(S) _STR(S)
#define _STR(S) #S


// Configuration variables.
double lat = LAT ;
double lon = LON ;


// Check the latitude.
int
validate_lat ( cfg_t* cfg, cfg_opt_t* opt )
  {
  float value = cfg_opt_getnfloat ( opt, 0 ) ;
  if ( value < -90.0 || value > +90.0 )
    {
    cfg_error ( cfg, "Invalid latitude." ) ;
    return -1 ;
    }
  return 0 ;
  }


// Check the longitude.
int
validate_lon ( cfg_t* cfg, cfg_opt_t* opt )
  {
  float value = cfg_opt_getnfloat ( opt, 0 ) ;
  if ( value < -180.0 || value > +180.0 )
    {
    cfg_error ( cfg, "Invalid longitude." ) ;
    return -1 ;
    }
  return 0 ;
  }


// Returns true if it's a leap year.
bool
isleapyear ( int y )
  {
  return ( ((y%4==0)&&(y%100!=0)) || (y%400==0) ) ;
  }


// Is this a valid date?
bool
valid_date ( int CY, int CM, int CD )
  {
  // Basic sanity check.
  if ( ( CD<1 || CD>31 ) || ( CM<1 || CM>12 ) ) return false ;
  // Gregorian calendar adopted in the UK, through to leap year error > 1day.
  if ( CY<1753 || CY>4582 ) return false ;
  // Nitty-gritty.
  switch ( CM )
    {
    // Thirty days hath September, April, June and November...
    case 9 : case 4 : case 6 : case 11 : if ( CD == 31 ) return false ; break ;
    // Numa Pompilius invented February, so it's his fault.
    case 2 :
      // Sweden had a February 30th. Once.
      if ( CD>29 ) return false ;
      if ( (CD>28) && (!isleapyear(CY)) ) return false ;
      break ;
    }
  return true ;
  }


// Show terse info.
void
usage ( char* appname )
  {
  // Get the time.
  time_t rawtime ; struct tm* gmt ;
  time ( &rawtime ) ; gmt = gmtime ( &rawtime ) ;
  printf ( "Usage: %s [-0|1|2|3] [--lat=%+07.3f --lon=%+08.3f] [%04d %02d %02d]\n",
    appname, lat, lon, gmt->tm_year+1900, gmt->tm_mon+1, gmt->tm_mday ) ;
  }


// Show version info.
void
version ( char* appname )
  {
  printf ( "%s v%s, W.B.Hill <mail@wbh.org>, 24 Sept 2021\n", appname, STR(VERSION) ) ;
  }


// Show some help.
void
help ( char* appname )
  {
  // Get the time.
  time_t rawtime ; struct tm* gmt ;
  time ( &rawtime ) ; gmt = gmtime ( &rawtime ) ;
  printf ( "Usage: %s [options] [YYYY MM DD]\n", appname ) ;
  printf ( "\t-h,--help        This help.\n" ) ;
  printf ( "\t-v,--version     The version info.\n" ) ;
  printf ( "\t   --la[titude]  Position, latitude, decimal degrees. (%+07.3f)\n", lat ) ;
  printf ( "\t   --lo[ngitude] Position, longitude, decimal degrees. (%+08.3f)\n", lon ) ;
  printf ( "\t-0               Calculate sunrise and sunset times. (default)\n" ) ;
  printf ( "\t-1               Calculate civil twiligh times.\n" ) ;
  printf ( "\t-2               Calculate nautical twilight times.\n" ) ;
  printf ( "\t-3               Calculate astronimical twilight times.\n" ) ;
  printf ( "\tDefault date: %04d %02d %02d\n", gmt->tm_year+1900, gmt->tm_mon+1, gmt->tm_mday ) ;
  printf ( "%s v%s, W.B.Hill <mail@wbh.org>, 24 Sept 2021\n", appname, STR(VERSION) ) ;
  }


// Sunrise, sunset and twilight times.
// From: https://www.edwilliams.org/sunrise_sunset_example.htm
int
suntimes
  (
  int zl ,
  double lat, double lon,
  int AY, int AM, int AD,
  int* Rh, int* Rm, int* Rs,
  int* Sh, int* Sm, int* Ss
  )
  {
  // Zenith.
  double zenith ;
  switch ( zl )
    {
    // Sunrise/sunset.
    case 0:
      zenith=90.0+50.0/60.0 ;
      break ;
    // Civil twilight.
    case 1:
      zenith=90.0+6.0 ;
      break ;
    // Nautical twilight.
    case 2:
      zenith=90.0+12.0 ;
      break ;
    // Astronomical twilight.
    case 3:
      zenith=90.0+18.0 ;
      break ;
    // Sunrise/sunset.
    default:
      zenith=90.0+50.0/60.0 ;
      break ;
    }
  // Day of the year.
  double N=floor(275.0*(double)AM/9.0)-(floor((AM+9.0)/12.0)*(1.0+floor((AY-4.0*floor(AY/4.0)+2.0)/3.0)))+AD-30.0 ;
  // Longitude hour, rising and setting.
  double tr=N+((6-lon/15.0)/24.0) ;
  double ts=N+((18.0-lon/15.0)/24.0) ;
  // Solar mean anomaly.
  double Mr=(0.9856*tr)-3.289 ;
  double Ms=(0.9856*ts)-3.289 ;
  // Solar true longitude.
  double Lr=Mr+(1.916*sin(Mr*M_PI/180.0))+(0.020*sin(2.0*Mr*M_PI/180.0))+282.634 ;
  if ( Lr<0.0) while (Lr<0.0) Lr+=360.0 ; if ( Lr>360.0) while (Lr>360.0) Lr-=360.0 ;
  double Ls=Ms+(1.916*sin(Ms*M_PI/180.0))+(0.020*sin(2.0*Ms*M_PI/180.0))+282.634 ;
  if ( Ls<0.0) while (Ls<0.0) Ls+=360.0 ; if ( Ls>360.0) while (Ls>360.0) Ls-=360.0 ;
  // Solar right ascension.
  double RAr=180.0*atan(0.91764*tan(Lr*M_PI/180.0))/M_PI ;
  if ( RAr<0.0) while (RAr<0.0) RAr+=360.0 ; if ( RAr>360.0) while (RAr>360.0) RAr-=360.0 ;
  RAr+=floor(Lr/90.0)*90.0-floor(RAr/90.0)*90.0;
  RAr/=15.0 ;
  double RAs=180.0*atan(0.91764*tan(Ls*M_PI/180.0))/M_PI ;
  if ( RAs<0.0) while (RAs<0.0) RAs+=360.0 ; if ( RAs>360.0) while (RAs>360.0) RAs-=360.0 ;
  RAs+=floor(Ls/90.0)*90.0-floor(RAs/90.0)*90.0;
  RAs/=15.0 ;
  // Solar declination.
  double Dr=180.0*asin(0.39782*sin(Lr*M_PI/180.0))/M_PI ;
  double Ds=180.0*asin(0.39782*sin(Ls*M_PI/180.0))/M_PI ;
  // Solar local hour angle, cosine.
  double cHr=(cos(zenith*M_PI/180.0)\
              -(sin(Dr*M_PI/180.0)*sin(lat*M_PI/180.0)))/(cos(Dr*M_PI/180.0)*cos(lat*M_PI/180.0)) ;
  double cHs=(cos(zenith*M_PI/180.0)\
              -(sin(Ds*M_PI/180.0)*sin(lat*M_PI/180.0)))/(cos(Ds*M_PI/180.0)*cos(lat*M_PI/180.0)) ;
  // cHr threshold should be +/- 1.000, but goes pathological.
  // Sun never rises?
  if ( cHr >= +0.995 ) return +1 ;
  // Sun never sets?
  if ( cHr <= -0.995 ) return -1 ;
  // Solar local hour angle.
  double Hr=(360.0-180.0*acos(cHr)/M_PI)/15.0 ;
  double Hs=(180.0*acos(cHs)/M_PI)/15.0 ;
  // Local mean times.
  double Tr=Hr+RAr-(0.06571*tr)-6.622 ;
  double Ts=Hs+RAs-(0.06571*ts)-6.622 ;
  // UTC.
  double UTr=Tr-lon/15.0 ;
  if ( UTr<0.0) while (UTr<0.0) UTr+=24.0 ; if ( UTr>24.0) while (UTr>24.0) UTr-=24.0 ;
  double UTs=Ts-lon/15.0 ;
  if ( UTs<0.0) while (UTs<0.0) UTs+=24.0 ; if ( UTs>24.0) while (UTs>24.0) UTs-=24.0 ;
  // Split it..
  int UTrH=(int)floor(UTr) ;
  int UTrM=(int)floor(60.0*(UTr-UTrH)) ;
  int UTrS=(int)floor(60.0*(60.0*(UTr-UTrH)-UTrM)) ;
  int UTsH=(int)floor(UTs) ;
  int UTsM=(int)floor(60.0*(UTs-UTsH)) ;
  int UTsS=(int)floor(60.0*(60.0*(UTs-UTsH)-UTsM)) ;
  // Return results.
  *Rh=UTrH; *Rm=UTrM; *Rs=UTrS;
  *Sh=UTsH; *Sm=UTsM; *Ss=UTsS;
  return 0 ;
  }


// Parse the config files, then the command line, setup and then finally do stuff.
int
main ( int argc, char* argv[] )
  {
  // Date to do.
  int AY, AM, AD ;
  // Sun times to calculate.
  int zenith = 0 ;
  // The end of the rope, err, string.
  char* end ;
  // Config file.
  static cfg_opt_t opts[] =
    {
    CFG_FLOAT ( "latitude", LAT, CFGF_NONE ),
    CFG_FLOAT ( "longitude", LON, CFGF_NONE ),
    CFG_END()
    } ;
  // Command line options.
  static struct option long_options[] =
    {
      { "help",      no_argument,       0,  'h' },
      { "version",   no_argument,       0,  'v' },
      { "latitude",  required_argument, 0,  'a' },
      { "longitude", required_argument, 0,  'o' },
      { 0, 0, 0, 0 }
    } ;
  // Load the config files.
  char* etcconf ;
  char* usrconf ;
  cfg_t* confuse ;
  // Generate the config file names from the app name.
  asprintf ( &etcconf, "/etc/%s.conf", basename ( argv[0] ) ) ;
  asprintf ( &usrconf, "%s/.%src", getenv ( "HOME" ), basename ( argv[0] ) ) ;
  // Init the config library.
  confuse = cfg_init ( opts, CFGF_NONE ) ;
  // Check the values.
  cfg_set_validate_func ( confuse, "latitude", validate_lat ) ;
  cfg_set_validate_func ( confuse, "longitude", validate_lon ) ;
  // Read the /etc/app.conf file.
  if ( cfg_parse ( confuse, etcconf ) == CFG_PARSE_ERROR )
    {
    fprintf ( stderr, "Problem with config file '%s'\n", etcconf ) ;
    exit ( EXIT_FAILURE ) ;
    }
  // Read the ~/.apprc file.
  if ( cfg_parse ( confuse, usrconf ) == CFG_PARSE_ERROR )
    {
    fprintf ( stderr, "Problem with config file '%s'\n", usrconf ) ;
    exit ( EXIT_FAILURE ) ;
    }
  // Save the values.
  lat = cfg_getfloat ( confuse, "latitude" ) ;
  lon = cfg_getfloat ( confuse, "longitude" ) ;
  // Done - free stuff.
  cfg_free ( confuse ) ;
  free ( etcconf ) ;
  free ( usrconf ) ;
  // Now parse the command line.
  int opt = 0 ;
  int long_index = 0 ;
  // Process the options.
  while ( ( opt = getopt_long ( argc, argv, "hv0123", long_options, &long_index ) ) != -1 )
    {
    switch ( opt )
      {
      case 'h' :
        help ( basename ( argv[0] ) ) ;
        exit ( EXIT_SUCCESS ) ;
      case 'v' :
        version ( basename ( argv[0] ) ) ;
        exit ( EXIT_SUCCESS ) ;
      case '0' :
        zenith = 0 ; break ;
      case '1' :
        zenith = 1 ; break ;
      case '2' :
        zenith = 2 ; break ;
      case '3' :
        zenith = 3 ; break ;
      case 'a' :
        lat = strtof ( optarg, &end ) ;
        if ( end == optarg )
          {
          fprintf ( stderr, "Invalid latitude!\n" ) ;
          exit ( EXIT_FAILURE ) ;
          }
        if ( lat < -90 || lat > +90 )
          {
          fprintf ( stderr, "Latitude out of range! : %f\n", lat ) ;
          exit ( EXIT_FAILURE ) ;
          }
        break ;
      case 'o' :
        lon = strtof ( optarg, &end ) ;
        if ( end == optarg )
          {
          fprintf ( stderr, "Invalid longitude!\n" ) ;
          exit ( EXIT_FAILURE ) ;
          }
        if ( lon < -180 || lon > +180 )
          {
          fprintf ( stderr, "Longitude out of range! : %f\n", lon ) ;
          exit ( EXIT_FAILURE ) ;
          }
        break ;
      default :
        usage ( basename ( argv[0] ) ) ;
        exit ( EXIT_FAILURE ) ;
      }
    }
  // Has a date been given?
  switch ( argc-optind )
    {
    time_t rawtime ; struct tm* gmt ;
    // No date, do today.
    case 0 :
      // Get the time.
      time ( &rawtime ) ; gmt = gmtime ( &rawtime ) ;
      AY = gmt->tm_year+1900 ; AM = gmt->tm_mon+1 ; AD = gmt->tm_mday ;
      break ;
    // Date given.
    case 3 :
      AY = (int) strtol ( argv[optind+0], (char **)NULL, 10 ) ;
      AM = (int) strtol ( argv[optind+1], (char **)NULL, 10 ) ;
      AD = (int) strtol ( argv[optind+2], (char **)NULL, 10 ) ;
      if ( !valid_date(AY,AM,AD) )
        {
        fprintf ( stderr, "Invalid date!\n" ) ;
        exit ( EXIT_FAILURE ) ;
        }
      break ;
    // Error.
    default :
      usage ( basename ( argv[0] ) ) ;
      exit ( EXIT_FAILURE ) ;
    }
  // Do it!
  int Rh, Rm, Rs ; int Sh, Sm, Ss ;
  int ret = suntimes ( zenith, lat, lon, AY, AM, AD, &Rh, &Rm, &Rs, &Sh, &Sm, &Ss ) ;
  switch ( ret )
    {
    case -1 :
      fprintf ( stdout, "NeverSets.\n" ) ;
      break ;
    case +1 :
      fprintf ( stdout, "NeverRises.\n" ) ;
      break ;
    case 0 :
      fprintf ( stdout, "%02d:%02d:%02d,%02d:%02d:%02d\n", Rh,Rm,Rs, Sh,Sm,Ss ) ;
      break ;
    default:
      fprintf ( stderr, "ERROR: Invalid suntimes() return value!\n" ) ;
      exit ( EXIT_FAILURE ) ;
    }
  // That's all, folks!
  return EXIT_SUCCESS ;
  }


// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
