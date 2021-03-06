/*
  DartLining.cpp

  generate 1D Poisson disk samples by placing them left to right
  with distance a uniform random variable in [r, 2r)

  Li-Yi Wei
  03/24/2010

*/

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <memory>
using namespace std;

#include <stdlib.h>
#include <math.h>

#include "Random.hpp"

#include "UniformDiff1d.hpp"
#include "GaussianDiff1d.hpp"

#include "Utility.hpp"
// #include "Math.hpp"
#include "Exception.hpp"

// this function assumes that a sample already exists in the origin
bool Full(const vector<float> & domain_size, const float r_value, const vector<Sample> & samples)
{
    if(samples.size() <= 0)
    {
        return 0;
    }
    else
    {
        const Sample last = samples[samples.size()-1];

        if(domain_size.size() != last.coordinate.Dimension())
        {
            throw Exception("dimension mistmatch");
        }

        for(int i = 0; i < last.coordinate.Dimension(); i++)
        {
            if((domain_size[i] - last.coordinate[i]) < 2*r_value)
            {
                return 1;
            }
        }

        return 0;
    }
}

int Main(int argc, char **argv)
{
    // input arguments
    const int argc_min = 6;

    if(argc < argc_min)
    {
        cerr << "Usage: " << argv[0] << " dimension (must be 1) num_class (must be 1) r_value (num_class numbers) domain_size (dimension numbers) diff_dist (uniform, gaussian?) " << endl;
        return 1;
    }

    int arg_ctr = 0;
    const int dimension = atoi(argv[++arg_ctr]);
    const int num_class = atoi(argv[++arg_ctr]);

    if(dimension != 1)
    {
        cerr << "dimension must be 1" << endl;
        return 1;
    }

    if(num_class != 1)
    {
        cerr << "can support only 1 class now" << endl;
        return 1;
    }

    if(argc < (argc_min + num_class - 1 + dimension - 1))
    {
        cerr << "not enough input arguments" << endl;
        return 1;
    }

    const float r_value = atof(argv[++arg_ctr]);

    vector<float> domain_size(dimension, 1.0);
    for(unsigned int i = 0; i < domain_size.size(); i++)
    {
        domain_size[i] = atof(argv[++arg_ctr]);
    }
    
    const string diff_dist = argv[++arg_ctr];

    // diff generator
    Diff1d * p_diff1d = 0;
    const string gaussian_string = "gaussian";
    const string gaussianp_string = "gaussianp";

    if(diff_dist == "uniform")
    {
        p_diff1d = new UniformDiff1d(r_value, 2*r_value);
    }
    else if(diff_dist.find(gaussian_string) == 0)
    {
        string std_string = "";

        if(diff_dist.find(gaussianp_string) == 0)
        {
            const string head = "0.";
            std_string = diff_dist.substr(gaussianp_string.length(), diff_dist.length());
            std_string.insert(std_string.begin(), head.begin(), head.end());
        }
        else
        {
            std_string = diff_dist.substr(gaussian_string.length(), diff_dist.length());
        }

        const float std = atof(std_string.c_str());

        if(std <= 0)
        {
            cerr << "gaussian std has to be > 0" << endl;
            return 1;
        }

        p_diff1d = new GaussianDiff1d(r_value, 2*r_value, std*r_value);
    }
    else
    {
        cerr << "unknown diff1d distribution" << endl;
        return 1;
    }

    if(!p_diff1d)
    {
        throw Exception("empty p_diff1d");
    }

    const Diff1d & diff1d = *p_diff1d;

    // init random
    Random::InitRandomNumberGenerator();

    // init samples
    vector<Sample> samples;
    Sample sample(dimension);
    sample.id = 0; // one class only
    sample.value = 1;

    // put one sample in origin
    for(int i = 0; i < sample.coordinate.Dimension(); i++)
    {
        sample.coordinate[i] = 0;
    }
    samples.push_back(sample);

    // generate the rest of the samples
    while(! Full(domain_size, r_value, samples))
    {
        if(samples.size() <= 0) throw Exception("weird");
        
        const Sample last = samples[samples.size()-1];
        
        bool out_of_bound = 0;

        for(int i = 0; i < sample.coordinate.Dimension(); i++)
        {
            const float min_coordinate = last.coordinate[i];
            sample.coordinate[i] = min_coordinate + diff1d.Diff();

            if((sample.coordinate[i] < 0) || (sample.coordinate[i] > domain_size[i]))
            {
                out_of_bound = 1;
            }
        }
        
        if(out_of_bound) break;

        samples.push_back(sample);
    }

    // shift all samples by a random amount

    // output
    // cerr << samples.size() << " samples" << endl;
    if(!Utility::WriteSamples(samples, "cout"))
    {
        cerr << "cannot write out samples" << endl;
        return 1;
    }

    // clean up
    if(p_diff1d)
    {
        delete p_diff1d;
        p_diff1d = 0;
    }

    // done
    return 0;
}

int main(int argc, char **argv)
{
    try
    {
        return Main(argc, argv);
    }
    catch(Exception e)
    {
        cerr << "Error : " << e.Message() << endl;
        return 1;
    }
}

