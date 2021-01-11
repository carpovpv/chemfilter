# ChemFilter - a cross-platform desktop application for ADMETox screening based on Transformer-CNN deep learning models

ChemFilter allows to view SDF files, estimate various physicochemical parameters and biological activities based on Transformer-CNN models (https://jcheminf.biomedcentral.com/articles/10.1186/s13321-020-00423-w). The program provides a filtering mechanism to select the most interesting compounds. To run it, you need neither GPU cards nor installed machine-learning frameworks. We offer all necessary files in the Releases for Windows environment. If you need a version for Mac OS or a particular version of Linux, please contact.

![Main window](https://github.com/carpovpv/chemfilter/blob/master/images/screenshot.png)

# Dependency
* Qt Framework, v. 5 (https://www.qt.io/)
* OpenBabel, v. 3 (http://openbabel.org/wiki/Main_Page)
* OpenBlas (https://www.openblas.net/)
* Transformer-CNN (https://github.com/bigchem/transformer-cnn). Only if you need to build a special QSAR/QSPR model.

# Building
The project governs by the CMake build system. 
To build ChemFilter, usual steps should be done:

1. cd chemfilter
2. mkdir bin 
3. cd bin 
4. cmake ../
5. make 
6. create s symlink to models folder to have access to ADEMTox models, ln -s ../models models

Now you can start the main Gui program.

# Usage 

A typical workflow is as follows:

1. Open an SDF file by pressing the corresponding button, "Open SDF." All the structures will be added to the main table. If the first is marked, the entire row will be exported (saved) in the output file. Unmarked rows will not be exported (saved). The second column always draws the structure itself; the third column is the SMILES representation of the molecule. All other columns may appear depending on properties that your SDF file might have. All these properties will be listed in the "Dataset Properties" panel. You can quickly delete some of them by pressing the corresponding button with the red circle. It is possible for columns with numerical values to see a simple histogram by pressing the second button (notepad and pencil). The histogram will appear below. The table supports sorting by pressing on the column header. Every column name is also a variable that can be used in scripts.

2. Calculate ADMETox (or other available) properties. To do this, select the models by marking them with "cross" (the mark could be different depending on your current desktop style). To select or deselect all models, use the corresponding buttons with Down and Up arrows, respectively. These buttons are on the "QSAR models panel." After selection, press the Run button. 

3. Every Transformer model adds two additional columns in the table—the column with the estimated property and the column with a relative error of estimation (%). For example, after calculating the LD50 model, the column LD50 will contain the assessment of LD50 values, and LD50_d will have a relative prognosis error. 

4. Now we can do some filtering. It is better to unselect all the table molecules by pressing the Up button (near the Export button). A filter script evaluates for every row and estimates whether the molecule passes or not. If it fails, the corresponding row is hidden (but not deleted). The script has to be written in QtScript language. Common filtering patterns include 50%, 75%, 80%, 90% and 95% likeness to DrugBank (https://go.drugbank.com) molecules. The filter will select only compounds, which predicted ADMETox properties in the interval for 50%, 75%, 80%, 90%, and 95% of DrugBank. For example, if a molecule passes a 75% filter, its ADMETox properties are in the same interval as 75% of all the market drugs. All the column names are also internal variable to the script. To filter, press the button with the green check. 

5. It is possible to add columns and fill them with values entered manually or based on a script. Press, "Add" button (plus icon) on the "Properties" panel. Type the column name without spaces or special symbols (this name will be used as a variable in scripts). The script may look like  Math.log(X) / Math.LN10. It will add a new column and calculate log10(X), where X is a column with caption X. Of course, this column must present. Otherwise, an error will be displayed. Every property cell's value is editable by double-clicking on it. 

6. Manually look through the compounds and mark those that are to be exported in a new file. 
7. Export the marked compounds and their corresponding properties (that are in the table) in a new file, by pressing the "Export" button. It has two options: export as SDF file, or as CSV. 

# Model definition 
All the models available for the program are in subdirectory models. Each property has its own directory with the property name as directory name. For example, files for solubility model are listed:

* info
* solubility-1.trm
* solubility-2.trm
* solubility-3.trm 

### Info file 
The info file contains information about the category, name, and DrugBank intervals for the model. Let's look into soulbility/info file.

```
Physicochemical properties
Water Solubility, log(mol/L)
solubility
50 -4.4749 -3.4006
75 -4.7887 -2.2597
80 -4.9028 -1.9365
90 -5.0739 -0.9382
95 -5.3876 -0.3297
```

Line # | Description | Example 
-------|-------------| -------
1| Category of the property. All the models are grouped into categories, for example: the toxicity category contains: Ames, hERG, and LD50, by default. | Physicochemical properties
2|The name of the model. This name is displayed in the model tree. | Water Solubility, log(mol/L)
3|The corresponding variable. When the model is about to run, it will ask the main table to create two additional columns for the results. | solubility (this column will have solubility estimation), solubility_d (relative error, %) 
4 and on|Typical interval for DrugBank compounds in form Percentage low high| 50 -4.4749 -3.4006. That means that estimations of the solubility of 50% of all compounds from the DrugBank lie in -4.4749 -3.4006. 

Only the first three lines are necessary.

### Converting Transformer-CNN models to binary format for ChemFilter
To build a custom model, the transformer-cnn original package has to be used (https://github.com/bigchem/transformer-cnn). There are two different script for RDkit and OpenBabel backends. You have to choose OpenBabel. The final model is placed in a tar archive file. To convert this tar file to binary trm file, a convert.py scipt should be used. It takes two arguments: the filename of the tar archive and the name of trm file to be generated. 

For example, you have developed a Transformer-CNN model for estimating pMIC values and your outpu file is pMIC.tar.

```
python3 convert.py pMIC.tar pMIC-1.trm 
```

You should also create a pMIC subfolder in the "models" directory and place pMIC-1.trm there. In case you wish to have multiple repetitions of the same model, you could name them pMIC-2.trm, pMIC-3.trm, etc. While making estimation, every model will contribute to the vectors of predicted values. Currently, 10 different augmented SMILES are used to make a prediction. So, if the subfolder contains only one trm file, then the vector will have 10 values. If 3 models are available, then the vector will have 30 estimations. The final value is to be put into the main table is the mean of all these values. The relative error is calculated based on all these values. Note that less than 10 different SMILES can be produced depending on a structure, and, consequently, the overall number of estimations will be lower, but still number_of_SMILES * number_of_models.

The info file could be as follows:

```
Custom models
pMIC
pmic
```
After you restart the ChemFilter program you have to see the model in the models tree.

# Prognosis console program 

The program reads all the models from the "models" directory. It reads SMILES from the stdin and outputs the properties calculated in JSON format to the stdout. Thus, it can be easily used in any scripting environment. For haloperidol, it has output similar to this:

{"ames": 5.99298e-06,"ames_d": 2,"caco2": 1.02705,"caco2_d": 10,"cyp2c9": 0.0276725,"cyp2c9_d": 91,"cyp3a4": 0.000754463,"cyp3a4_d": 66,"dmso": 0.998374,"dmso_d": 0,"fu": -1.13989,"fu_d": 4,"herg": 0.570661,"herg_d": 34,"hia": 0.995536,"hia_d": 1,"hob": 0.915962,"hob_d": 16,"ld50": 3.33052,"ld50_d": 3,"logd": 2.99685,"logd_d": 4,"metabolic_stability": 0.0224747,"metabolic_stability_d": 116,"pgp_inhibitor": 0.999658,"pgp_inhibitor_d": 0,"pgp_substrate": 0.408574,"pgp_substrate_d": 62,"pka": 8.60061,"pka_d": 2,"ppb": 1.02707,"ppb_d": 8,"solubility": -4.37149,"solubility_d": 2,"vod": 1.09984,"vod_d": 7}


#  To-do list
1. possibility to load and merge several SDF files; 
2. adding new rows (molecules);
3. possibility to calculate a similarity measure to compounds in external files;
4. enhancing the histogram plot;
5. visualization of atoms' contributions, LRP.

In case of interest, do not hesitate to contact me.




