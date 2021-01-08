# ChemFilter - a cross-platform desktop application for ADMETox screening based on Transformer-CNN deep learning models

ChemFilter allows to view SDF files, estimate various physicochemical parameters and biological activities based on Transformer-CNN models (https://jcheminf.biomedcentral.com/articles/10.1186/s13321-020-00423-w). The program provides a filtering mechanism to select the most interesting compounds. To run it, you need neither GPU cards nor installed machine-learning frameworks. We offer all necessary files in the Releases for Windows environment. If you need a version for Mac OS or a particular version of Linux, please contact us.

![Main window](https://github.com/carpovpv/chemfilter/blob/master/images/screenshot.png)

# Dependency
* Qt Framework, v. 5 (https://www.qt.io/)
* OpenBabel, v. 3 (http://openbabel.org/wiki/Main_Page)
* OpenBlas (https://www.openblas.net/)

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
4 and on|Typical interval for DrugBank compounds in form Percentage low high| 50 -4.4749 -3.4006. That means that estimations of the solubility of 50% of all compounds from the DrugBank lie in -4.4749 -3.4006



