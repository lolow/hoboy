/*
 *      hoboy.cpp
 *
 *      Copyright 2009 Laurent Drouet <ldrouet@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 *
 *      HOBOY : a command line tool for the OBOE solver
 *
 */

//Command Line includes
#include <tclap/CmdLine.h>
#include <tclap/ValuesConstraint.h>

//Standard includes
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

//OBOE includes
#include "Parameters.h"
#include "Oracle.h"
#include "QpGenerator.h"
#include "AccpmDynMatrix.h"
#include "AccpmBlasInterface.h"

using namespace std;
using namespace TCLAP;
using namespace Accpm;

class GenericOracleFunction : public OracleFunction {

private:

// Read a vector from a file
void readVector(const string &fileName, AccpmVector &vector) {

    ifstream fin( fileName.c_str() );

    if ( fin.is_open() ) {

      string line;
      istringstream instream;
      double val;
      for (int i = 0; i < vector.size(); ++i) {

        if (getline(fin, line)) {

            instream.clear();
            instream.str(line);

            if ((instream >> val)) {

                vector(i) = val;

            } else {

                cerr << "Error reading vector from file: " << fileName;
                cerr << " at line:" << i+1 << endl;
            }
        } else {

            cerr << "Error reading vector from file: " << fileName << endl;
        }
      }

    } else {

      cerr << "Error opening file: " << fileName << endl;
      exit(1);

    }
}

// Read a matrix from a file
void readMatrix(const string &fileName, AccpmGenMatrix &matrix) {

    ifstream fin( fileName.c_str() );

    if ( fin.is_open() ) {

      string line;
      istringstream instream;
      double val;
      for (int i = 0; i < matrix.size(0); ++i) {

        if (getline(fin, line)) {

            instream.clear();
            instream.str(line);

            for (int j = 0; j < matrix.size(1); ++j) {

                if ((instream >> val >> ws)) {

                    matrix(i, j) = val;

                } else {

                    cerr << "Error reading matrix from file: " << fileName;
                    cerr << " at line:" << i+1 << " column: " << j + 1 << endl;
                }
            }
        } else {

            cerr << "Error reading matrix from file: " << fileName << std::endl;
        }
      }

    } else {

      std::cerr << "Error opening file: " << fileName << std::endl;
      exit(1);

    }
}

public:
    string functionFilename;
    string gradientFilename;
    string cutFilename;
    virtual int eval(const AccpmVector &y,
                           AccpmVector &functionValue,
                           AccpmGenMatrix &subGradients,
                           AccpmGenMatrix *info){

        // Function Value;
        readVector(functionFilename, functionValue);

        // Subgradient Value
        readMatrix(gradientFilename, subGradients);

        // Optimality cut
        readMatrix(cutFilename, *info);

        return 0;
    };
};

// Read a vector from a file
void readVector(const string &fileName, vector<double> &vector) {

    ifstream fin( fileName.c_str() );

    if ( fin.is_open() ) {

      string line;
      istringstream instream;
      double val;
      for (unsigned int i = 0; i < vector.size(); ++i) {

        if (getline(fin, line)) {

            instream.clear();
            instream.str(line);

            if ((instream >> val)) {

                vector.at(i) = val;

            } else {

                cerr << "Error reading vector from file: " << fileName;
                cerr << " at line:" << i+1 << endl;
            }
        } else {

            cerr << "Error reading vector from file: " << fileName << endl;
        }
      }

    } else {

      cerr << "Error opening file: " << fileName << endl;
      exit(1);

    }
}

int main(int argc, char** argv)
{
    try{

        // Command line parameters

        CmdLine cmd("A command line tool for the OBOE solver", ' ', "0.1");

        vector<string> allowedAction;
		allowedAction.push_back("init");
		allowedAction.push_back("oracle");
		ValuesConstraint<string> allowedActionVals( allowedAction );

        ValueArg<std::string> actionArg   ("a","action","Action to perform",true,"init",&allowedActionVals);
        ValueArg<std::string> saveArg     ("t","state","Name of the state/save file",false,"oboe.asc","file");
        ValueArg<std::string> parameterArg("p","param","Name of the parameter file",false,"param.txt","file");
        ValueArg<std::string> functionArg ("f","func","Name of the function file",false,"func.txt","file");
        ValueArg<std::string> gradientArg ("g","grad","Name of the subgradient file",false,"grad.txt","file");
        ValueArg<std::string> cutArg      ("c","cut","Name of the cut file",false,"cut.txt","file");
        ValueArg<std::string> errorArg    ("e","error","Name of the error log file",false,"","file");
        ValueArg<std::string> outputArg   ("o","output","Name of the output file",false,"","file");
        ValueArg<std::string> lowerArg    ("l","lowerbounds","Name of the lower bounds file",false,"","file");
        ValueArg<std::string> upperArg    ("u","upperbounds","Name of the upper bounds file",false,"","file");
        ValueArg<std::string> startArg    ("s","start","Name of the starting point file",false,"","file");

        cmd.add( lowerArg );
        cmd.add( upperArg );
        cmd.add( startArg );
        cmd.add( errorArg );
        cmd.add( outputArg );
        cmd.add( cutArg );
        cmd.add( gradientArg );
        cmd.add( functionArg );
        cmd.add( saveArg );
        cmd.add( parameterArg );
        cmd.add( actionArg );

        cmd.parse( argc, argv );

        // Get the value parsed by each arg.
        string action            = actionArg.getValue();
        string saveFilename      = saveArg.getValue();
        string parameterFilename = parameterArg.getValue();
        string functionFilename  = functionArg.getValue();
        string gradientFilename  = gradientArg.getValue();
        string cutFilename       = cutArg.getValue();
        string errorFilename     = errorArg.getValue();
        string outputFilename    = outputArg.getValue();
        string lowerbFilename    = lowerArg.getValue();
        string upperbFilename    = upperArg.getValue();
        string startFilename     = startArg.getValue();

        // Redirect cerr to a file
        ofstream errorFile( errorFilename.c_str() );
        streambuf * cerr_buffer = cerr.rdbuf();
        if( errorFilename != "" ){
            cerr.rdbuf(errorFile.rdbuf());
        }

        if (action=="init"){

            // Read OBOE parameter file
            Parameters param ( parameterFilename.c_str() );

            //Starting point and bounds
            int n = param.getIntParameter("NumVariables");
            if ( startFilename != "" ){
                vector<double> start(n, 0);
                readVector(startFilename,start);
                param.setStartingPoint(start);
            }
            if ( lowerbFilename != "" ){
                vector<double> varLB(n, 0);
                readVector(lowerbFilename,varLB);
                param.setVariableLB(varLB);
            }
            if ( upperbFilename != "" ){
                vector<double> varUB(n, 0);
                readVector(upperbFilename,varUB);
                param.setVariableUB(varUB);
            }

            // OBOE initialisation
            GenericOracleFunction f1;
            Oracle oracle(&f1);
            QpGenerator qpGen;
            qpGen.init(&param, &oracle);

            // Save state
            qpGen.save( saveFilename.c_str() );

            if ( outputFilename != "" ){

                ofstream f ( outputFilename.c_str() );
                f << *qpGen.getQueryPoint();
                f.close();

            } else {
                cout << *qpGen.getQueryPoint();
            }

        }

        if (action=="oracle"){

            // Read OBOE parameter file
            Parameters param ( parameterFilename.c_str() );

            //Starting point and bounds
            int n = param.getIntParameter("NumVariables");
            if ( lowerbFilename != "" ){
                vector<double> varLB(n, 0);
                readVector(lowerbFilename,varLB);
                param.setVariableLB(varLB);
            }
            if ( upperbFilename != "" ){
                vector<double> varUB(n, 0);
                readVector(upperbFilename,varUB);
                param.setVariableUB(varUB);
            }

            // OBOE initialisation
            GenericOracleFunction f1;
            Oracle oracle(&f1);
            QpGenerator qpGen;
            qpGen.init(&param, &oracle);

            // Load state
            qpGen.load( saveFilename.c_str() );

            //Define filenames for oracle
            f1.functionFilename = functionFilename;
            f1.gradientFilename = gradientFilename;
            f1.cutFilename      = cutFilename;

            //OBOE iteration
            qpGen.run();

            // save state
            qpGen.save( saveFilename.c_str() );

            if ( outputFilename != "" ){

                ofstream f ( outputFilename.c_str() );
                f << qpGen.getExitCode() << endl;
                f << qpGen.getRelativeGap() << endl;
                f << *qpGen.getQueryPoint();
                f.close();

            } else {
                cout << qpGen.getExitCode() << endl;
                cout << qpGen.getRelativeGap() << endl;
                cout << *qpGen.getQueryPoint();
            }

        }

        // Redirect cerr to stderr
        if(errorFilename!=""){
            cerr.rdbuf(cerr_buffer);
        }

    	} catch (ArgException &e)  // catch any exceptions
	{ cerr << "error: " << e.error() << " for arg " << e.argId() << endl; }

}
