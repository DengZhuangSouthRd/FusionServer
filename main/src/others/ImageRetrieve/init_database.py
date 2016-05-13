# -*- coding: utf-8 -*-
"""
Created on Wed May 11 16:10:51 2016

@author: fan
"""
import os
import os.path


import psycopg2
path="f:\imageretrievedata\photo"

foldername=''
targetno=0
targetname=''
filename=''
t=()
folderlist=os.listdir(path)
for folder in folderlist:
    foldername=folder
    t=foldername.split('_')
    targetno=t[0]
    targetname=t[1]
    print targetno
    
    print targetname
    
    conn=psycopg2.connect(database="testdb",user="postgres",password="csuduc",host="10.2.3.119",port="5432")
    cur=conn.cursor()
    
    cur.execute("INSERT INTO t3targetinfo(targetno,targetname)VALUES(%s,%s)",(targetno,targetname.encode("utf8")))
   
    conn.commit()
    cur.close()
    oldpath=path+"\\"+folder
    filelist=os.listdir(oldpath)
    for file in filelist:
        newpath=path+'\\'+folder+"\\"
        print newpath
        filename=file
        print filename
        cur=conn.cursor()
        cur.execute("INSERT INTO t4pic(targetno,targetname,picpath,picname)VALUES(%s,%s,%s,%s)",(targetno,targetname.encode("utf8"),newpath.encode("utf8"),filename))
        conn.commit()
        cur.close()
    
    

#conn=psycopg2.connect(database="testdb",user="postgres",password="csuduc",host="10.2.3.119",port="5432")
#cur=conn.cursor()

#cur.execute("INSERT INTO t4pic(picpath,picname,targetname,targetno,featurepath,dicpath,status_)VALUES(%s,%s,%s,%s,%s,%s,%s)",('f','P','f',1,'f','f',2));
#conn.commit()
#cur.close()
#print "success"


