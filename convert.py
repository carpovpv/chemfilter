#convert trans-cnn archives to binary models
#compatible with chemfiler

import h5py
import numpy as np 
import pickle
import sys 
import struct
import os
import tarfile 

tar = tarfile.open(sys.argv[1]);
tar.extractall();
tar.close();

mdl = pickle.load( open("model.pkl", "rb"));
mdltype = mdl[0][2];

if mdltype == "regression" :
   mdllow = mdl[0][3];
   mdlhigh = mdl[0][4];

#combine weights from embeddings.npy and model.h5py into
#one pickle list for convinience

DD = [];
if mdltype == "regression":
   DD.append( [mdltype, mdllow, mdlhigh]);
else:
   DD.append( [mdltype]);

d = np.load("embeddings.npy", allow_pickle = True);
for q in d:
   DD.append(q);

f = h5py.File("model.h5", "r");

w = f["conv1d_6"]["conv1d_6"]["kernel:0"][:][0];
b = f["conv1d_6"]["conv1d_6"]["bias:0"][:];

DD.append(w);
DD.append(b);

#usual valid convolutions
w = f["conv1d_7"]["conv1d_7"]["kernel:0"][:];
b = f["conv1d_7"]["conv1d_7"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_8"]["conv1d_8"]["kernel:0"][:];
b = f["conv1d_8"]["conv1d_8"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_9"]["conv1d_9"]["kernel:0"][:];
b = f["conv1d_9"]["conv1d_9"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_10"]["conv1d_10"]["kernel:0"][:];
b = f["conv1d_10"]["conv1d_10"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_11"]["conv1d_11"]["kernel:0"][:];
b = f["conv1d_11"]["conv1d_11"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_12"]["conv1d_12"]["kernel:0"][:];
b = f["conv1d_12"]["conv1d_12"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_13"]["conv1d_13"]["kernel:0"][:];
b = f["conv1d_13"]["conv1d_13"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_14"]["conv1d_14"]["kernel:0"][:];
b = f["conv1d_14"]["conv1d_14"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_15"]["conv1d_15"]["kernel:0"][:];
b = f["conv1d_15"]["conv1d_15"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_16"]["conv1d_16"]["kernel:0"][:];
b = f["conv1d_16"]["conv1d_16"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["conv1d_17"]["conv1d_17"]["kernel:0"][:];
b = f["conv1d_17"]["conv1d_17"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["dense_3"]["dense_3"]["kernel:0"][:];
b = f["dense_3"]["dense_3"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["dense_4"]["dense_4"]["kernel:0"][:];
b = f["dense_4"]["dense_4"]["bias:0"][:];

DD.append(w);
DD.append(b);

w = f["dense_5"]["dense_5"]["kernel:0"][:];
b = f["dense_5"]["dense_5"]["bias:0"][:];

DD.append(w);
DD.append(b);

if mdltype == "regression":
   w = f["Regression-property"]["Regression-property"]["kernel:0"][:];
   b = f["Regression-property"]["Regression-property"]["bias:0"][:];
else:
   w = f["Classification-property"]["Classification-property"]["kernel:0"][:];
   b = f["Classification-property"]["Classification-property"]["bias:0"][:];

DD.append(w);
DD.append(b);

#This was actually another program... 
#But adjust all the indexies is a little bit tricky...

info = DD[0];
inf = np.zeros(len(info), np.float32);

if info[0] == "regression":
   inf[0] = 0;
else:
   inf[0] = 1;

if len(info)  == 3:
   inf[1] = info[1];
   inf[2] = info[2];

d = DD[1:];

DD = [];
DD.append(inf);

DD.append(d[0]);

K = [];
V = [];
Q = [];
   
for block in range(10):
   K.append(d[1 + 3*block]);
   V.append(d[2 + 3*block]);
   Q.append(d[3 + 3*block]);

K = np.concatenate(K, axis=1);
Q = np.concatenate(Q, axis=1);
V = np.concatenate(V, axis=1);

DD.append(K);
DD.append(Q);
DD.append(V);
   
DD.append(d[31]);
DD.append(d[32]);
DD.append(d[33]);
DD.append(d[34]);
DD.append(d[35][0]);
DD.append(d[36]);
DD.append(d[37][0]);
DD.append(d[38]);
DD.append(d[39]);
DD.append(d[40]);

K = [];
V = [];
Q = [];

for block in range(10):
   K.append(d[41 + 3*block]);
   V.append(d[42 + 3*block]);
   Q.append(d[43 + 3*block]);

K = np.concatenate(K, axis=1);
Q = np.concatenate(Q, axis=1);
V = np.concatenate(V, axis=1);

DD.append(K);
DD.append(Q);
DD.append(V);

DD.append(d[71]);
DD.append(d[72]);
DD.append(d[73]);
DD.append(d[74]);
DD.append(d[75][0]);
DD.append(d[76]);
DD.append(d[77][0]);
DD.append(d[78]);
DD.append(d[79]);
DD.append(d[80]);
 
K = [];
Q = [];
V = [];

for block in range(10):
   K.append(d[81 + 3*block]);
   V.append(d[82 + 3*block]);
   Q.append(d[83 + 3*block]);

K = np.concatenate(K, axis=1);
Q = np.concatenate(Q, axis=1);
V = np.concatenate(V, axis=1);

DD.append(K);
DD.append(Q);
DD.append(V);

DD.append(d[111]);
DD.append(d[112]);
DD.append(d[113]);
DD.append(d[114]);
DD.append(d[115][0]);
DD.append(d[116]);
DD.append(d[117][0]);
DD.append(d[118]);
DD.append(d[119]);
DD.append(d[120]); 
DD.append(d[121]);
DD.append(d[122]);

DD.append(d[123] [:, :].reshape((-1, 200)));   
DD.append(d[124]);
DD.append(d[125] [:, :].reshape((-1, 200)));
DD.append(d[126]);
DD.append(d[127] [:, :].reshape((-1, 200)));
DD.append(d[128]);
DD.append(d[129] [:, :].reshape((-1, 200)));
DD.append(d[130]);
DD.append(d[131] [:, :].reshape((-1, 100)));
DD.append(d[132]);
DD.append(d[133] [:, :].reshape((-1, 100)));
DD.append(d[134]);
DD.append(d[135] [:, :].reshape((-1, 100)));
DD.append(d[136]);
DD.append(d[137] [:, :].reshape((-1, 100)));
DD.append(d[138]);
DD.append(d[139] [:, :].reshape((-1, 100)));
DD.append(d[140]);
DD.append( d[141] [:, :].reshape((-1, 160)));
DD.append(d[142]);
DD.append( d[143] [:, :].reshape((-1, 160)));
DD.append(d[144]);

DD.append(d[145]);
DD.append(d[146]);
DD.append(d[147]);
DD.append(d[148]);

DD.append(d[149]);
DD.append(d[150]);
DD.append(d[151]);
DD.append(d[152]);

#dump matrixes
fp = open(sys.argv[2], "wb");
for e in DD:
   r = e.flatten();
   myfmt='f'*len(r);
   bin=struct.pack(myfmt,*r)
   fp.write(bin)

fp.close();

os.remove("model.pkl");
os.remove("model.h5");
os.remove("embeddings.npy");

