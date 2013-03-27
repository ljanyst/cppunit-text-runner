//------------------------------------------------------------------------------
// Copyright (c) 2013 by European Organization for Nuclear Research (CERN)
// Author: Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------
// CppUnitTextRunner is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CppUnitTextRunner is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with XppUnitTextRunner.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <sys/time.h>
#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <dlfcn.h>
#include <cppunit/Test.h>
#include <cppunit/TestSuite.h>
#include <cppunit/Exception.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestPath.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>

//------------------------------------------------------------------------------
// Get timestamp in microseconds
//------------------------------------------------------------------------------
uint64_t GetTimeStamp()
{
  struct timeval  tv;
  struct timezone tz;
  gettimeofday( &tv, &tz );
  return 1000000 * tv.tv_sec + tv.tv_usec;
}

//------------------------------------------------------------------------------
// Format time
//------------------------------------------------------------------------------
std::string FormatTime( uint64_t t )
{
  std::ostringstream o;
  o << std::setw(6) << std::setfill(' ') << t/1000000 << ".";
  o << std::setw(6) << std::setfill('0') << t%1000000;
  return o.str();
}

//------------------------------------------------------------------------------
// Print the progress
//------------------------------------------------------------------------------
class ProgressPrinter: public CppUnit::TestListener
{
  public:
    ProgressPrinter( std::ostream &stream ):
      pIsFailure( false ),
      pTestStart( 0 ),
      pStream( stream ) {}

    virtual	~ProgressPrinter() {}

    virtual void startTest( CppUnit::Test *test )
    {
      pIsFailure = false;
      pTestStart = GetTimeStamp();
      pStream << "[i] Running " << std::setw( 50 ) << std::left;
      pStream << test->resolveTestPath("").toString() + "...";
      pStream << std::flush;
    }

    void PrintElapsed( uint64_t start )
    {
      uint64_t elapsed = GetTimeStamp() - start;
      pStream << "[" << FormatTime( elapsed ) << "s]";
    }

    virtual void addFailure( const CppUnit::TestFailure &failure )
    {
      pIsFailure = true;
      CppUnit::SourceLine l = failure.sourceLine();
      PrintElapsed( pTestStart );
      pStream << " [FAILED]" << std::endl;
      pStream << "----------" << std::endl;
      pStream << failure.thrownException()->what() << std::endl;
      pStream << "Where: " << l.fileName() << ":" << l.lineNumber();
      pStream << std::endl;
      pStream << "----------" << std::endl;
    }

    virtual void endTest( CppUnit::Test *test )
    {
      if( pIsFailure )
        return;
      PrintElapsed( pTestStart );
      pStream << " [OK]" << std::endl;
    }

  private:
    bool          pIsFailure;
    uint64_t      pTestStart;
    std::ostream &pStream;
};

//------------------------------------------------------------------------------
// Print all the tests present in the test suite
//------------------------------------------------------------------------------
void printTests( const CppUnit::Test *t, std::string prefix = "" )
{
  if( t == 0 )
    return;

  const CppUnit::TestSuite *suite = dynamic_cast<const CppUnit::TestSuite*>( t );
  std::cerr << prefix << t->resolveTestPath("").toString();
  if( suite )
  {
    std::cerr << std::endl;
    std::string prefix1 = "  "; prefix1 += prefix;
    const std::vector<CppUnit::Test*> &tests = suite->getTests();
    std::vector<CppUnit::Test*>::const_iterator it;
    for( it = tests.begin(); it != tests.end(); ++it )
      printTests( *it, prefix1 );
  }
  else
    std::cerr << std::endl;
}

//------------------------------------------------------------------------------
// Start the show
//------------------------------------------------------------------------------
int main( int argc, char **argv)
{
  //----------------------------------------------------------------------------
  // Load the test library
  //----------------------------------------------------------------------------
  if( argc < 2 )
  {
    std::cerr << "Usage: " << argv[0] << " libname.so testname" << std::endl;
    return 1;
  }
  void *libHandle = dlopen( argv[1], RTLD_LAZY );
  if( libHandle == 0 )
  {
    std::cerr << "Unable to load the test library: " << dlerror() << std::endl;
    return 1;
  }

  //----------------------------------------------------------------------------
  // Print help
  //----------------------------------------------------------------------------
  CppUnit::Test *all = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
  if( argc == 2 )
  {
    std::cerr << "Select your tests:" << std::endl << std::endl;
    printTests( all );
    std::cerr << std::endl;
    return 1;
  }

  std::string testPath = argv[2];

  //----------------------------------------------------------------------------
  // Set up the test runner
  //----------------------------------------------------------------------------
  CppUnit::TestResult controller;

  CppUnit::TestResultCollector result;
  controller.addListener( &result );

  ProgressPrinter progress( std::cerr );
  controller.addListener( &progress );

  //----------------------------------------------------------------------------
  // Run the tests
  //----------------------------------------------------------------------------
  CppUnit::TestRunner runner;
  runner.addTest( all );
  try
  {
    uint64_t suiteStart = GetTimeStamp();
    std::cerr << "[i] Running " << testPath << "..." << std::endl;
    runner.run( controller, testPath );
    uint64_t elapsed = GetTimeStamp() - suiteStart;
    std::cerr << "[i] Done with, time elapsed " << testPath;
    std::cerr << FormatTime( elapsed ) << "s" << std::endl;
  }
  catch ( std::invalid_argument &e )  // Test path not resolved
  {
    std::cerr << "[!] " << e.what() << std::endl;
    return 2;
  }

  dlclose( libHandle );
  return result.wasSuccessful() ? 0 : 1;
}
