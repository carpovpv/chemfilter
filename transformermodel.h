/*

   Transformer Screening Project.

   P. Karpov, 2020
   carpovpv@gmail.com

*/ 

#ifndef TRANSFORMERMODEL_H
#define TRANSFORMERMODEL_H

#include <map>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>

#include <openbabel/mol.h>

#include <cblas.h>

//Generate a batch of SMILES corresponding to the molecule.
//If the mol is out of vocabulary, or some problem in RDKIT occured
//the function returns false.

bool GetRandomSmiles(const std::string & smiles, 
                     std::set<std::string> & mols, 
                     int &max_n);

class TransformerModel
{

public:

    static const int MaxBatchSize  =  10;  //No more than 10 augmented SMILES per molecule.                                             
    static const int MaxSmilesSize = 128;  //The length of a SMILE (characters, not atoms).

    //These constants come from the Transformer-CNN article.
    static const int EmbeddingSize =  64;   
    static const int HeadsCount    =  10;
    static const int ConvOffset    =  20;
    static const int HiddenSize    = 512;

    static constexpr char const * vocab = " ^#%()+-./0123456789=@ABCDEFGHIKLMNOPRSTVXYZ[\\]abcdefgilmnoprstuy$";
    static const int vocab_length = 66;  //The length of the vocabulary string.
    
    TransformerModel(const char * fname);
    ~TransformerModel();

    struct ResultValue
    {
         bool valid;
         double value[TransformerModel::MaxBatchSize];
         int size;

         ResultValue()
         {
             valid = false;
         }
    };

    bool isGood() const;
    ResultValue predict(std::set<std::string> & mols, int max_n);

private:

    //Mapping from a symbol to wordId.
    std::map<char, int> char_to_ix;

    int left_mask_id [ MaxBatchSize ];
    int x[MaxBatchSize * (MaxSmilesSize + ConvOffset)];

    char * data; //model
    bool classification;
    double v_min;
    double v_max;

    //Variables for matrixes during the calculations.   
    double * smiles_embedding;
    double * pos;

    double *q, *k, *v;
    double *a, *sa, *lc;

    //Variables for the model loaded.
    double *mdl;

    //Attention layers.
    double * K1[3], *Q1[3], *V1[3], *TD1[3], *B1[3];
    double * gamma1[3], *beta1[3], *w1[3], *b1[3], *w2[3], *b2[3];
    double * gamma2[3], *beta2[3];

    //Convolutional filters.
    double * Conv1,  * Conv1_B;
    double * Conv2,  * Conv2_B;
    double * Conv3,  * Conv3_B;
    double * Conv4,  * Conv4_B;
    double * Conv5,  * Conv5_B;
    double * Conv6,  * Conv6_B;
    double * Conv7,  * Conv7_B;
    double * Conv8,  * Conv8_B;
    double * Conv9,  * Conv9_B;
    double * Conv10, * Conv10_B;
    double * Conv15, * Conv15_B;
    double * Conv20, * Conv20_B;

    //HighWay module.
    double * CNN_W, * CNN_WB;
    double * High1, * High1_B;
    double * High2, * High2_B;

    //The final output;
    double * Out_W;
    double * Out_B;
    
    struct ConvInfo
    {
        int conv_number;
        int n_filter;
        double * conv;    // W
        double * bias;    // B
        int start;        // Start position of this filter in the lc array.
    };

    double *ccc;

};

#endif // TRANSFORMERMODEL_H

