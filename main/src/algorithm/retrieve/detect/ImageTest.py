# -*- coding: utf-8 -*-

import os,csv,pickle
import numpy as np
import scipy.io as sio
from sklearn.linear_model import OrthogonalMatchingPursuit

# txt data load
def loadTxt(filename):
    if os.path.exists(filename) == False:
        print "data file path not exits !"
        return None
    dataArray = []
    with open(filename,'rb') as handle:
        for line in handle:
            res = [float(x) for x in line.split(",")]
            dataArray.append(res)
    return np.array(dataArray)

# mat data load  
def loadMat(filename):
    if os.path.exists(filename) == False:
        print "data file path not exits !"
        return None
    data = sio.loadmat(filename)
    return data["D"]
    
def loadPkl(filename):
    if os.path.exists(filename) == False:
        print "data file path not exits !"
        return None
    handle = open(filename,'rb')
    data = pickle.load(handle)
    handle.close()
    return data

def loadDictionary(filename,kind = "mat"):
    if kind == "mat":
        return loadMat(filename)
    if kind == "pkl":
        return loadPkl(filename)

def calcPercentage(label_info):
    dict_label = {}
    for x in label_info:
        if x not in dict_label.keys():
            dict_label[x] = 1
        else:
            dict_label[x] = dict_label[x] + 1
    res = []
    for item in dict_label.keys():
        res.append((item,dict_label[item]*1.0 / len(label_info)))
    return sorted(res,key = lambda x:x[1],reverse=True)

def sparse_Asift(dicData, imgFeature, n_none_zero, n_class):
    if n_class > len(dicData):
        raise IndexError
        return None
    dic = dicData[0][0]
    dic_index = [dicData[0][0].shape[1]]
    for i in xrange(1,n_class):
        dic = np.hstack((dic,dicData[i][0]))
        dic_index.append(dic_index[i-1] + dicData[i][0].shape[1])

    imgTranspose = imgFeature.T
    omp = OrthogonalMatchingPursuit(n_nonzero_coefs=n_none_zero)
    omp.fit(dic,imgFeature.T)
    coef = omp.coef_
    residual = np.zeros((n_class))
    min_index = np.zeros((imgFeature.shape[0]))

    for i in xrange(imgFeature.shape[0]):
        y_std = imgTranspose[:,i]
        w = coef[i,0:dic_index[0]]
        x = dicData[0][0]
        y = np.dot(x,w)
        #print y_std.shape,y.shape
        residual[0] = np.linalg.norm(y_std - y)
        for j in xrange(1,n_class):
            tmp_w = coef[i,dic_index[j-1]:dic_index[j]]
            tmp_x = dicData[j][0]
            #print tmp_x.shape,tmp_w.shape
            tmp_y = np.dot(tmp_x,tmp_w)
            residual[j] = np.linalg.norm(y_std - tmp_y)
        #min_val = residual.min()
        min_index[i] = residual.argmin() + 1
    return min_index,calcPercentage(min_index)

def imgRecognition(filename_txt, filename_mat, saveDir, n_none_zero=9, class_num = 40):
    testData = loadTxt(filename_txt)
    dicData = loadDictionary(filename_mat, kind = 'mat')
    label_res = sparse_Asift(dicData,testData[:,2:],n_none_zero,class_num)
    print label_res[1][0:5]

if __name__ == "__main__":
    filename_txt = u"F:/Python_Guiyang/test-ASIFT-DUCD-0406.txt"
    filename_mat = u"F:/Python_Guiyang/Dictionary_features-ASIFT-DUCD.mat"
    saveDir = u"F:/Python_Guiyang/"
    imgRecognition(filename_txt, filename_mat, saveDir, class_num = 7)
