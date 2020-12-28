/*

   Transformer Screening Project.

   P. Karpov, 2020
   carpovpv@gmail.com

*/ 

#ifndef TRANSFORMERMODEL_H
#define TRANSFORMERMODEL_H

#include <map>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>

#include <openbabel/mol.h>

#include <cblas.h>

//Generate a batch of SMILES corresponding to the molecule.
//If the mol is out of vocabulary, or some problem occured
//the function returns false.

bool GetRandomSmiles(const std::string & smiles, 
                     std::set<std::string> & mols, 
                     int &max_n);
//< 30 points
float student(int freedom);

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

    static const char * vocab;
    static const int vocab_length = 66;  //The length of the vocabulary string.
    
    TransformerModel(const char * fname, const char * prop = NULL);
    ~TransformerModel();

    struct ResultValue
    {
         bool valid;
         float value[TransformerModel::MaxBatchSize];
         int size;

         ResultValue()
         {
             valid = false;
         }
    };

    bool isGood() const;
    ResultValue predict(std::set<std::string> & mols, int max_n,
                        float * embeddings = NULL);

    float * getSmilesEmbeddings();
    void setSmilesEmbeddings(float * s);

    const std::string & getProp() const;

private:

    std::string m_prop;

    //Mapping from a symbol to wordId.
    std::map<char, int> char_to_ix;    

    int left_mask_id [ MaxBatchSize ];
    int x[MaxBatchSize * (MaxSmilesSize + ConvOffset)];

    char * data; //model
    bool classification;
    float v_min;
    float v_max;

    //Variables for matrixes during the calculations.   
    float * smiles_embedding;
    float * pos;

    float *q, *k, *v;
    float *a, *sa, *lc;

    //Variables for the model loaded.
    float *mdl;

    //Attention layers.
    float * K1[3], *Q1[3], *V1[3], *TD1[3], *B1[3];
    float * gamma1[3], *beta1[3], *w1[3], *b1[3], *w2[3], *b2[3];
    float * gamma2[3], *beta2[3];

    //Convolutional filters.
    float * Conv1,  * Conv1_B;
    float * Conv2,  * Conv2_B;
    float * Conv3,  * Conv3_B;
    float * Conv4,  * Conv4_B;
    float * Conv5,  * Conv5_B;
    float * Conv6,  * Conv6_B;
    float * Conv7,  * Conv7_B;
    float * Conv8,  * Conv8_B;
    float * Conv9,  * Conv9_B;
    float * Conv10, * Conv10_B;
    float * Conv15, * Conv15_B;
    float * Conv20, * Conv20_B;

    //HighWay module.
    float * CNN_W, * CNN_WB;
    float * High1, * High1_B;
    float * High2, * High2_B;

    //The final output;
    float * Out_W;
    float * Out_B;

    int NN;
    int batch_size;
    
    struct ConvInfo
    {
        int conv_number;
        int n_filter;
        float * conv;    // W
        float * bias;    // B
        int start;       // Start position of this filter in the lc array.
    };

    void AttentionLayer(int layer);
};

#endif // TRANSFORMERMODEL_H

