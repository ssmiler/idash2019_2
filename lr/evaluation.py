#!/usr/bin/python

import pandas as pd
import numpy as np
from sklearn.metrics import *
import numpy as np
import matplotlib.pyplot as plt
import sys, getopt

def f(x):
    return np.exp(x)/(1+np.exp(x))

# Compute ROC curve and ROC area for each class
def getStats(y_label,y_pred):
    fpr = dict()
    tpr = dict()
    roc_auc = dict()
    n_classes = y_pred.shape[1]
    for i in range(n_classes):
        fpr[i], tpr[i], thres = roc_curve(y_label[:, i], y_pred[:, i])
        roc_auc[i] = auc(fpr[i], tpr[i])

    # Compute micro-average ROC curve and ROC area
    fpr["micro"], tpr["micro"], _ = roc_curve(y_label.ravel(), y_pred.ravel())
    roc_auc["micro"] = auc(fpr["micro"], tpr["micro"])
    return fpr, tpr, roc_auc, n_classes

# Plot ROC curve
def plotROC(n_classes, roc_auc, fpr, tpr,title, outputfile):
    plt.figure()
    plt.plot(fpr["micro"], tpr["micro"],
             label='micro-average ROC curve (area = {0:0.6f})'
                   ''.format(roc_auc["micro"]))
    for i in range(n_classes):
        plt.plot(fpr[i], tpr[i], label='ROC curve of class {0} (area = {1:0.6f})'
                                       ''.format(i, roc_auc[i]))

    plt.plot([0, 1], [0, 1], 'k--')
    plt.xlim([0.0, 1.0])
    plt.ylim([0.0, 1.05])
    plt.xlabel('False Positive Rate')
    plt.ylabel('True Positive Rate')
    plt.title(title)
    plt.legend(loc="lower right")
    plt.savefig(outputfile)


def main(argv):
    inputfile =''
    outputfile =''
    try:
        opts, args = getopt.getopt(argv,"hi:t:o:",["ifile=","tfile=","ofile="])
    except getopt.GetoptError:
        print('python evaluation.py -i <inputfile> -t <targetfile> -o <outputfile>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('python evaluation.py -i <inputfile> -t <targetfile> -o <outputfile>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-t", "--tfile"):
            targetfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
    
    print('Input file is: ', inputfile)
    print('Target file is: ', targetfile)
    print('Output file is: ', outputfile)
    
    
   
    ref = pd.read_csv(inputfile)
    target = pd.read_csv(targetfile)
     
    # for reference data
    print(20*"-"+"MicroAUC for reference data"+20*"-")

    y_label_ref = pd.get_dummies(target['class'].values).reindex(columns=[0,1,2], fill_value=0).values
    y_pred_ref = ref.iloc[0:,2:].values

    
    fpr_ref, tpr_ref, roc_auc_ref, n_classes_ref = getStats(y_label_ref, y_pred_ref)
    print('MicroAUC for %d classes ' % n_classes_ref)
    print(roc_auc_ref)
    
    plotROC(n_classes_ref, roc_auc_ref, fpr_ref, tpr_ref,'ROC curves for reference data', outputfile)

if __name__ == "__main__":
    main(sys.argv[1:])
    