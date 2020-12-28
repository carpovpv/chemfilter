/*

   Transformer Screening Project.

   P. Karpov, 2020
   carpovpv@gmail.com

*/

#include "transformermodel.h"

#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <boost/algorithm/string.hpp>

#if not __APPLE__
    #include <omp.h>
#endif

float student(int freedom)
{
    //two-sided 95%
    static const float tbl []= {
        -1.0,
        12.71, 4.303, 3.182, 2.776, 2.571,
        2.447, 2.365, 2.306, 2.262, 2.228,
        2.201, 2.179, 2.160, 2.145, 2.131,
        2.120, 2.110, 2.101, 2.093, 2.086,
        2.080, 2.074, 2.069, 2.064, 2.060,
        2.056, 2.052, 2.048, 2.045, 2.042
    };

    if(freedom > 30)
        return 1.960;

    return tbl [freedom];
}

const char * TransformerModel::vocab = " ^#%()+-./0123456789=@ABCDEFGHIKLMNOPRSTVXYZ[\\]abcdefgilmnoprstuy$";

TransformerModel::TransformerModel(const char *fname, const char *prop)
{
    data = NULL;
    q   = NULL;
    mdl = NULL;

    if(prop)
       m_prop = prop;

    FILE  * fp = fopen(fname, "rb");
    if(fp == NULL)
    {
        fprintf(stderr, "Error opening file's model.\n");
        return;
    }

    if(fseek(fp, 0, SEEK_END))
    {
        fclose(fp);
        fprintf(stderr, "Error determining the size (fseek) of the file's model.\n");

        return;
    }

    long size = ftell(fp);
    if(size == -1)
    {
        fclose(fp);
        fprintf(stderr, "Error determining the size (ftell) of the file's model.\n");

        return;
    }

    if(fseek(fp, 0, SEEK_SET))
    {
        fclose(fp);
        fprintf(stderr, "Error rewinding the file's model.\n");

        return;
    }

    data = (char *) malloc( size );
    if(data == NULL)
    {
        fclose(fp);
        fprintf(stderr, "Memory allocation error.\n");

        return;
    }    

    ssize_t rcnt = fread(data, 1, size, fp);
    if(rcnt != size)
    {
        fclose(fp);
        free(data);
        fprintf(stderr, "Error reading the model.\n");

        data = NULL;
        return;
    }
 
    fclose(fp);    

    float * pdata = (float *)data;
    classification = (pdata[0] == 1.0);
    if(!classification)
    {
        v_min = pdata[1];
        v_max = pdata[2];

        mdl = pdata + 3;

        //printf("Borders: %g %g\n", v_min, v_max);
    }
    else
        mdl = pdata + 1;

    //We alloc once for all matrixes in the model. Then we use pointers to different 
    //memory locations devoted to specific matrixies and results.

    q = (float *) malloc(
        (10 * (MaxSmilesSize + ConvOffset) * EmbeddingSize * HeadsCount  // Q, V, K in the SelfAttention Module
         + (MaxSmilesSize + ConvOffset) * (MaxSmilesSize + ConvOffset)  // attention coefficients
         + (MaxSmilesSize + ConvOffset) * EmbeddingSize * 200           // output of the convolutional filter
         + 1720                                                         // otput of the MaxPool layer
         + (MaxSmilesSize + ConvOffset) * EmbeddingSize                 // positional encodings
         + (MaxSmilesSize + ConvOffset) * EmbeddingSize                 // vocab encodings
         ) * MaxBatchSize * sizeof(float));
    
    if ( q == NULL)
    {
         free(data);
         fprintf(stderr, "Memory allocation error.\n");

         data = NULL;
         return;       
    }

    k   = q  + (MaxSmilesSize + ConvOffset) * EmbeddingSize * HeadsCount * MaxBatchSize;
    v   = k  + (MaxSmilesSize + ConvOffset) * EmbeddingSize * HeadsCount * MaxBatchSize;
    a   = v  + (MaxSmilesSize + ConvOffset) * EmbeddingSize * HeadsCount * MaxBatchSize;
    sa  = a  + (MaxSmilesSize + ConvOffset) * (MaxSmilesSize + ConvOffset) * MaxBatchSize;
    lc  = sa + (MaxSmilesSize + ConvOffset) * EmbeddingSize * 200 * MaxBatchSize;  //200 max number of filters
    pos = lc + 1720 * MaxBatchSize; 
    smiles_embedding = pos + (MaxSmilesSize + ConvOffset) * EmbeddingSize * MaxBatchSize;
 
    //Positional encoding matrix.
    for(int j=0; j< MaxSmilesSize + ConvOffset; j++)    
        for(int i=0; i< EmbeddingSize; i++)
        {
            if(i % 2 == 0)
                pos[ j * EmbeddingSize + i] = sin( (j + 1.0) / pow(10000.0, float(i) / EmbeddingSize));
            else
                pos[ j * EmbeddingSize + i] = cos( (j + 1.0) / pow(10000.0, float(i-1) / EmbeddingSize));
        }
    

    //First layer
    K1[0] = mdl + vocab_length * EmbeddingSize;
    Q1[0] = K1[0] + EmbeddingSize * EmbeddingSize * HeadsCount;
    V1[0] = Q1[0] + EmbeddingSize * EmbeddingSize * HeadsCount;
    TD1[0] = V1[0] + EmbeddingSize * EmbeddingSize * HeadsCount;
    B1[0] = TD1[0] + EmbeddingSize * EmbeddingSize * HeadsCount;

    gamma1[0] = B1[0] + EmbeddingSize;
    beta1[0] = gamma1[0] + EmbeddingSize;
    w1[0] = beta1[0] + EmbeddingSize;
    b1[0] = w1[0] + EmbeddingSize * HiddenSize;
    w2[0] = b1[0] + HiddenSize;
    b2[0] = w2[0] + EmbeddingSize * HiddenSize;
    gamma2[0] = b2[0] + EmbeddingSize;
    beta2[0] = gamma2[0] + EmbeddingSize;

    //Second layer
    K1[1] = beta2[0] + EmbeddingSize;
    Q1[1] = K1[1] + EmbeddingSize * EmbeddingSize * HeadsCount;
    V1[1] = Q1[1] + EmbeddingSize * EmbeddingSize * HeadsCount;
    TD1[1] = V1[1] + EmbeddingSize * EmbeddingSize * HeadsCount;
    B1[1] = TD1[1] + EmbeddingSize * EmbeddingSize * HeadsCount;

    gamma1[1] = B1[1] + EmbeddingSize;
    beta1[1] = gamma1[1] + EmbeddingSize;
    w1[1] = beta1[1] + EmbeddingSize;
    b1[1] = w1[1] + EmbeddingSize * HiddenSize;
    w2[1] = b1[1] + HiddenSize;
    b2[1] = w2[1] + EmbeddingSize * HiddenSize;
    gamma2[1] = b2[1] + EmbeddingSize;
    beta2[1] = gamma2[1] + EmbeddingSize;

    //Third layer
    K1[2] = beta2[1] + EmbeddingSize;
    Q1[2] = K1[2] + EmbeddingSize * EmbeddingSize * HeadsCount;
    V1[2] = Q1[2] + EmbeddingSize * EmbeddingSize * HeadsCount;
    TD1[2] = V1[2] + EmbeddingSize * EmbeddingSize * HeadsCount;
    B1[2] = TD1[2] + EmbeddingSize * EmbeddingSize * HeadsCount;

    gamma1[2] = B1[2] + EmbeddingSize;
    beta1[2] = gamma1[2] + EmbeddingSize;
    w1[2] = beta1[2] + EmbeddingSize;
    b1[2] = w1[2] + EmbeddingSize * HiddenSize;
    w2[2] = b1[2] + HiddenSize;
    b2[2] = w2[2] + EmbeddingSize * HiddenSize;
    gamma2[2] = b2[2] + EmbeddingSize;
    beta2[2] = gamma2[2] + EmbeddingSize;

    Conv1 = beta2[2] + EmbeddingSize;  Conv1_B = Conv1 + EmbeddingSize * 100;

    Conv2 =  Conv1_B  + 100;  Conv2_B =  Conv2  + 128  * 200;
    Conv3 =  Conv2_B  + 200;  Conv3_B =  Conv3  + 192  * 200;
    Conv4 =  Conv3_B  + 200;  Conv4_B =  Conv4  + 256  * 200;
    Conv5 =  Conv4_B  + 200;  Conv5_B =  Conv5  + 320  * 200;
    Conv6 =  Conv5_B  + 200;  Conv6_B =  Conv6  + 384  * 100;
    Conv7 =  Conv6_B  + 100;  Conv7_B =  Conv7  + 448  * 100;
    Conv8 =  Conv7_B  + 100;  Conv8_B =  Conv8  + 512  * 100;
    Conv9 =  Conv8_B  + 100;  Conv9_B =  Conv9  + 576  * 100;
    Conv10 = Conv9_B  + 100;  Conv10_B = Conv10 + 640  * 100;
    Conv15 = Conv10_B + 100;  Conv15_B = Conv15 + 960  * 160;
    Conv20 = Conv15_B + 160;  Conv20_B = Conv20 + 1280 * 160;

    CNN_W = Conv20_B + 160;
    CNN_WB = CNN_W + 1720 * 512;

    High1 = CNN_WB + 512;
    High1_B = High1 + 512 * 512;

    High2 = High1_B + 512;
    High2_B = High2 + 512 * 512;

    Out_W = High2_B + 512;
    Out_B = Out_W + 512;

    for(int i=0; i < vocab_length; i++)
        char_to_ix[ vocab[i] ] = i;

}

TransformerModel::~TransformerModel()
{ 
    if(data)
        free(data);
    if(q)
        free(q);
}

bool TransformerModel::isGood() const
{
    return (q != NULL && data != NULL);
}

TransformerModel::ResultValue TransformerModel::predict(std::set < std::string > & mols, int N,
                                                        float *embeddings)
{
    struct ConvInfo cv_info[12] = {
         { 1, 100,  Conv1,  Conv1_B,    0},
         { 2, 200,  Conv2,  Conv2_B,  100},
         { 3, 200,  Conv3,  Conv3_B,  300},
         { 4, 200,  Conv4,  Conv4_B,  500},
         { 5, 200,  Conv5,  Conv5_B,  700},
         { 6, 100,  Conv6,  Conv6_B,  900},
         { 7, 100,  Conv7,  Conv7_B, 1000},
         { 8, 100,  Conv8,  Conv8_B, 1100},
         { 9, 100,  Conv9,  Conv9_B, 1200},
         {10, 100, Conv10, Conv10_B, 1300},
         {15, 160, Conv15, Conv15_B, 1400},
         {20, 160, Conv20, Conv20_B, 1560},
    };

    ResultValue fin;
    fin.valid = true;

    //Real batch size may differ form the max due to peculiarities of mols.
    batch_size = mols.size();

    //Adjustment for the max of conv windows.
    NN = N + ConvOffset;
    int i_mol = 0;

    if(embeddings == NULL)
    {

        std::memset(smiles_embedding, 0, (MaxSmilesSize + ConvOffset) * MaxBatchSize * sizeof(float));
        std::memset(x, 0, (ConvOffset + MaxSmilesSize) * MaxBatchSize * sizeof(int) );

        for(std::set<std::string>::const_iterator it = mols.begin();
            it != mols.end(); ++it)
        {
            for(size_t i=0; i< (*it).size(); i++)
               x[i_mol * NN + i] = char_to_ix[ (*it)[i] ];

            left_mask_id[i_mol] = (*it).size();
            i_mol++;
        }

        //Extract Embeddings
        #pragma omp parallel for
        for(int i_mol = 0; i_mol < batch_size; i_mol++)
        {
           for (int i=0; i< NN; ++i)
              std::memcpy(&smiles_embedding[i_mol * NN * EmbeddingSize + i * EmbeddingSize],
                        &mdl[x[i_mol * NN + i] * EmbeddingSize], EmbeddingSize * sizeof(float) );
           //Add Position.
           cblas_saxpy(left_mask_id[i_mol] * EmbeddingSize, 1.0, pos, 1,
                       &smiles_embedding[i_mol * NN * EmbeddingSize], 1);
        }

        AttentionLayer(0);
        AttentionLayer(1);
        AttentionLayer(2);
    }
    else
        smiles_embedding = embeddings;

    //smile_embedding variable contains the final embeddings of the molecule
    //next step is convolutional filters

    //We do not have relu here, because afterwards there is max function.
    std::memset(&lc[0], 0, sizeof(float) * 1720 * batch_size);

    for(int conv= 0; conv < 12; ++conv)
    {
        const int nn = NN - cv_info[conv].conv_number +1;

        #pragma omp parallel for collapse(2)
        for(int i_mol=0; i_mol < batch_size; i_mol++)
        for(int i=0; i< nn; i++)
        {
            std::memcpy(&q[ i_mol * nn * EmbeddingSize * cv_info[conv].conv_number +
                            i*EmbeddingSize * cv_info[conv].conv_number],
                        &smiles_embedding[i_mol * NN * EmbeddingSize + i*EmbeddingSize],
                        EmbeddingSize * cv_info[conv].conv_number * sizeof(float) );
        }


        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    nn * batch_size,
                    cv_info[conv].n_filter,
                    EmbeddingSize * cv_info[conv].conv_number, 1.0,
                    q, EmbeddingSize * cv_info[conv].conv_number,
                    cv_info[conv].conv, cv_info[conv].n_filter, 0.0,
                    sa, cv_info[conv].n_filter);

        #pragma omp parallel for collapse(2)
        for(int i_mol=0; i_mol < batch_size; i_mol++)
        for(int i=0; i< nn; i++)
        {
            float * value = &sa[i_mol * nn * cv_info[conv].n_filter +
                                 i * cv_info[conv].n_filter + 0];
            float * bias = cv_info[conv].bias;
            float * ll = &lc[i_mol * 1720 + cv_info[conv].start];

            for(int j=0; j<cv_info[conv].n_filter; ++j)
            {
                *value += *bias;
                if(*ll < *value) *ll = *value;
                value++, ll++, bias++;
            }
        }
    }

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, batch_size,
                512, 1720, 1.0, lc, 1720, CNN_W, 512, 0.0, k, 512);

    //#pragma omp parallel for collapse(2)
    for(int i_mol=0; i_mol < batch_size; i_mol++)
       for(int i=0; i < 512; i++)
       {
           k[i_mol * 512 + i] += CNN_WB[i];
           if(k[i_mol * 512 + i] < 0.0) k[i_mol * 512 + i] = 0.0;
       }

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, batch_size,
                512, 512, 1.0, k, 512, High1,
                512, 0.0, q, 512);

    #pragma omp parallel for collapse(2)
    for(int i_mol=0; i_mol < batch_size; i_mol++)
    for(int i=0; i<512; ++i)
    {
        q[i_mol * 512 + i] = 1.0 / (1.0 + expf(-q[i_mol * 512 + i] - High1_B[i]));
        v[i_mol * 512 + i] = 1.0 - q[i_mol * 512 + i];
    }

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, batch_size,
                512, 512, 1.0, k, 512, High2, 512, 0.0, sa, 512);

    for(i_mol=0; i_mol < batch_size; i_mol++)
    {
        fin.value[i_mol] = Out_B[0];
        for(int i=0; i<512; ++i)
        {
            if(sa[i_mol * 512 + i] <= -High2_B[i])
                fin.value[i_mol] += Out_W[i] * v[i_mol * 512 +i] * k[i_mol * 512 +i];
            else
                fin.value[i_mol] += Out_W[i] * ( v[i_mol * 512 +i] * k[i_mol * 512 +i]
                        + (sa[i_mol * 512 +i] + High2_B[i]) * q[i_mol * 512 +i]);
        }
        if(classification)
            fin.value[i_mol] = 1.0 / (1.0 + expf(-fin.value[i_mol]));
        else
            fin.value[i_mol]  = (fin.value[i_mol]  - 0.9) / 0.8 * (v_max - v_min) + v_max;
    }
    fin.size = batch_size;
    return fin;
}

float * TransformerModel::getSmilesEmbeddings()
{
    return smiles_embedding;
}

void TransformerModel::setSmilesEmbeddings(float *s)
{
    smiles_embedding = s;
}

const std::string &TransformerModel::getProp() const
{
    return m_prop;
}

void TransformerModel::AttentionLayer(int layer)
{

    #pragma omp parallel sections
    {
       #pragma omp section
           cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN * batch_size,
                    EmbeddingSize * HeadsCount, EmbeddingSize, 1.0, smiles_embedding, EmbeddingSize, Q1[layer],
                    EmbeddingSize * HeadsCount, 0.0, q, EmbeddingSize * HeadsCount);

       #pragma omp section
           cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN * batch_size,
                    EmbeddingSize * HeadsCount, EmbeddingSize, 1.0, smiles_embedding, EmbeddingSize, K1[layer],
                    EmbeddingSize * HeadsCount, 0.0, k, EmbeddingSize * HeadsCount);

       #pragma omp section
           cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN * batch_size,
                    EmbeddingSize * HeadsCount, EmbeddingSize, 1.0, smiles_embedding, EmbeddingSize, V1[layer],
                    EmbeddingSize * HeadsCount, 0.0, v, EmbeddingSize * HeadsCount);
    }

    for(int i_mol = 0; i_mol < batch_size; i_mol++)
    {
         for (int head= 0; head< HeadsCount; ++head)
         {

            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, NN,
                        NN, EmbeddingSize, 1.0,
                        &q[ i_mol * NN * HeadsCount * EmbeddingSize + EmbeddingSize * head],
                        EmbeddingSize * HeadsCount,
                        &k[ i_mol * NN * HeadsCount * EmbeddingSize + EmbeddingSize * head],
                        EmbeddingSize * HeadsCount, 0.0, a, NN);

            #pragma omp parallel for
            for(int i= 0; i < NN; i++)
            {
                float *s = &a[i*NN];

                //In batch case mask might be different!
                std::memset(s + left_mask_id[i_mol], 0,
                            (NN - left_mask_id[i_mol]) * sizeof(float));

                float S = 0.0;
                for(int l =0; l< left_mask_id[i_mol]; l++)
                {
                    *s = expf( *s / 8.0);
                    S += *s++;
                }

                for(int l =0; l< left_mask_id[i_mol]; l++)
                    *--s /= S;
            }

            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN,
                        EmbeddingSize, NN, 1.0, a, NN,
                        &v[i_mol * NN * HeadsCount * EmbeddingSize + EmbeddingSize * head],
                        EmbeddingSize * HeadsCount, 0.0,
                        &sa[i_mol * NN * HeadsCount * EmbeddingSize + EmbeddingSize * head],
                        EmbeddingSize * HeadsCount);
        }

    }

    #pragma omp parallel for
    for(int i=0 ; i < NN * batch_size; i++)
       std::memcpy(&a[i * EmbeddingSize], B1[layer], EmbeddingSize * sizeof(float) );

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN * batch_size,
                EmbeddingSize, EmbeddingSize * HeadsCount, 1.0, sa, EmbeddingSize * HeadsCount, TD1[layer],
                EmbeddingSize, 1.0, a, EmbeddingSize);

    cblas_saxpy(NN * EmbeddingSize * batch_size, 1.0, smiles_embedding, 1, a, 1);

    #pragma omp parallel for collapse(2)
    for(int i_mol=0; i_mol < batch_size; i_mol++)
    for(int i= 0; i< NN; ++i)
    {

        float s1 = 0;
        float s2 = 0;

        float *s = &a[i_mol * EmbeddingSize * NN + i*EmbeddingSize];
        for(int j=0; j< EmbeddingSize; j++)
        {
             s2 += (*s) * (*s);
             s1 += *s++;
        }

        float Mean = s1 / EmbeddingSize;
        s1 = sqrt(s2 - s1 * Mean)/8.0 + 1e-6;

        for(int j= EmbeddingSize -1; j>=0; j--)
        {
            s--;
            *s = (*s - Mean) * gamma1[layer][j] / s1 + beta1[layer][j];
        }
    }

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN * batch_size,
                HiddenSize, EmbeddingSize, 1.0, a, EmbeddingSize, w1[layer],
                HiddenSize, 0.0, q, HiddenSize);

    #pragma omp parallel for collapse(2)
    for(int i_mol=0; i_mol < batch_size; i_mol++)
    for(int i= 0; i< NN; ++i)
    {
        float * p = &b1[layer][0];
        float * s = &q[i_mol * NN * HiddenSize + i*HiddenSize];

        for(int j=0; j < HiddenSize; j++)
        {
            *s += *p++;
            if(*s < 0) *s = 0.0;
            s++;
        }
        std::memcpy(&k[i_mol * NN * EmbeddingSize + i* EmbeddingSize], b2[layer],
                EmbeddingSize * sizeof(float));
    }

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NN * batch_size,
                EmbeddingSize, HiddenSize, 1.0, q, HiddenSize, w2[layer],
                EmbeddingSize, 1.0, k, EmbeddingSize);

    cblas_saxpy(NN * EmbeddingSize * batch_size, 1.0, k, 1, a, 1);

    #pragma omp parallel for collapse(2)
    for(int i_mol=0; i_mol < batch_size; i_mol++)
    for(int i=0; i<NN; ++i)
    {
        float s1 = 0;
        float s2 = 0;

        float * s = &a[i_mol * EmbeddingSize * NN + i*EmbeddingSize];
        for(int j=0; j< EmbeddingSize; j++)
        {
             s2 += (*s) * (*s);
             s1 +=  *s++;
        }

        float Mean = s1 / EmbeddingSize;
        s1 = sqrt(s2 - s1 * Mean)/8.0 + 1e-6;

        for(int j= EmbeddingSize-1;j >=0; j--)
             smiles_embedding[i_mol * NN * EmbeddingSize + i*EmbeddingSize + j] = (*--s - Mean) *
                     gamma2[layer][j] / s1 + beta2[layer][j];
    }

};

bool GetRandomSmiles(const std::string & smiles, 
                     std::set<std::string> & mols,
                     int & max_n)
{
  
  mols.clear();

  if(smiles.size() > TransformerModel::MaxSmilesSize)
     return false;

  //If we have a symbol that is not in our vocabulary, 
  //reject the molecule from further calculations.

  for(size_t i=0; i< smiles.size(); i++)
     if(!std::strchr(TransformerModel::vocab, smiles[i])) 
         return false;

  mols.insert(smiles);
  max_n = smiles.size();

  OpenBabel::OBConversion conv;
  conv.SetInFormat("SMI");
  conv.SetOutFormat("SMI");
  conv.AddOption("C");

  std::stringstream s_in (smiles);
  conv.SetInStream(&s_in);

  OpenBabel::OBMol m;
  conv.Read(&m);
  m.StripSalts();

  if(m.NumAtoms() > 0)
  {
      for(int i=0; i< 100; i++)
      {
          std::stringstream s_out;

          conv.SetOutStream(&s_out);
          conv.Write(&m);

          std::string augm = s_out.str();
          boost::trim(augm);

          if(augm.size() > TransformerModel::MaxSmilesSize)
             continue;

          if(mols.find(augm) == mols.end())
          {
             if(max_n < (int)augm.size())
                max_n = augm.size();

             mols.insert(augm);
          }

          if(mols.size() >= TransformerModel::MaxBatchSize)
             break;
      }
  }

  return mols.size();
}
