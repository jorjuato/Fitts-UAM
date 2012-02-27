#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import numpy
import pylab
from mpl_toolkits.axes_grid.parasite_axes import HostAxes, ParasiteAxes
from scipy.fftpack import hilbert 
from scipy.optimize import leastsq


dataPath = "./data"
grPath = "./figs"
grExt = ".png"

dt = {'names':   ('x',  'MinX','MaxX', 'y',  'vx', 'vy', 'press','time', 'side', 'osc', 'errors','ID'),\
      'formats': ('f4', 'f4',  'f4',   'f4', 'f4', 'f4', 'u2'   ,'f4',   'S1',   'u1',  'u1',    'f4')}
hFmt = {'ID0':'f',
        'IDf':'f',
        'initSide':'s',
        'OBS':'i',
        'ODS':'i',
        'OAS':'i',
        'SlopeTime':'f', 
        'replications':'i'}
cols = (0,1,2,3,4,5,6,7,8,9,10,11)
rows = 2
eps2 = 0.1
eps3 = 200
eps  = 0.05

########################################################################
############        MAIN   #############################################
########################################################################
def main():
    rawData = load_data()
    mtData = getMTPhase(rawData)
    plotAll(mtData)
    return rawData, mtData

def plotAll(mtData):
    plotMTInd(mtData)
    plotMTCombined(mtData)
    plotMTCombinedDouble(mtData)
    
########################################################################
############        GET MOVEMENT TIME FUNCTIONS   ######################
########################################################################

def getMTPhase(res):
    resall = dict()     
    for subject, data in res.iteritems():
        indRes = dict()
        #Store indivual based data to average replications 
        for i, trial in enumerate(data):
            print "Procesando MT para el sujeto:%s, trial No = %d out of %d" % (subject, i+1, len(data))
            os.sys.stdout.flush()
            #Load data temporal series and configuration of trial
            tr = load_trial(trial)
            if tr == None: continue

            #Get indivual continous time series of MT     
            retval = getMTContinous(tr)
            if retval == None: continue
            mt, time, id = retval[:]
            idS = getIDShift(tr)
            
            ##Get sigmoidal fitting
            #guess = numpy.array([mt[0],mt[-1],mt[0],0.0005,idS,0.5])
            #p = fitSigmoid(time,mt,guess)
            
            #Store results in dictionary
            if tr['conf'] in indRes.keys():
                indRes[tr['conf']].append( filterOutliers(time,mt,idS, id))
            else:
                indRes[tr['conf']] = [ filterOutliers(time,mt, idS,id) ]
                
        #Concatenate indivual based data to average replications      
        for conf, trialType in indRes.iteritems():
            mt = numpy.empty(0)
            t  = numpy.empty(0)
            id = numpy.empty(0)
            idSList = numpy.array([rep[2] for rep in trialType ])
            idSMin = idSList.min()  #All other signal will offsetted wrt this
            #Align the different trials at ID shift time
            for rep in trialType: 
                tS = rep[2]-idSMin              #Get temporal offset
                t_ = rep[1] -tS                 #Shift time array
                idx =  numpy.where(rep[1]>0)    #Remove samples with negative time
                mt = numpy.concatenate((mt,rep[0][idx]))
                t  = numpy.concatenate((t,t_[idx]))
                id = numpy.concatenate((id,rep[3][idx]))
            #Stored filtered combined data
            indRes[conf] = {'all':filterOutliers(t,mt,idSMin,id), 'ind':indRes[conf]}
        resall[subject] = indRes.copy() 
    return resall

def getMTContinous(trial):
    mtList = list()
    time = list()
    idList = list()
    ph = trial['ph']
    t = trial['t']
    id = trial['id']
    for now in range(len(ph)):
        #Generate index and mask arrays of in-phase points
        idx = numpy.where(abs(ph - ph[now]) < eps)[0]
        idx_mask = numpy.diff(idx) > 10
        idx_mask = numpy.insert(idx_mask,0,False)
        
        #Look for the next phase repetition downstream
        flag = False  
        for i in range(now-1,-1,-1):
            if idx_mask[numpy.where(idx==i)] == True:
                before = i
                flag = True
                break
        if flag == False: continue
        
        #Look for the next phase repetition upstream
        flag = False  
        for j in range(now,len(ph)):
            if idx_mask[numpy.where(idx==j)] == True:
                after = j
                flag = True
                break
        if flag == False: continue
        
        #Save to list
        mtList.append((t[after] - t[before])/2)
        time.append(t[now])
        idList.append(id[now])

    #Discard poor trials and Filter outliers  
    if len(mtList) < 50 :
        return None
    else:
        mtArr = numpy.array(mtList)
        tmArr = numpy.array(time)
        idArr = numpy.array(idList)
        #filter = numpy.where(abs(mtArr -mtArr.mean()) < 2*mtArr.std())
        #return mtArr[filter], tmArr[filter]
        return mtArr, tmArr, idArr
   
def getMTOscillation(oscArr, ID, time):
    MTlist = list()
    idx = 0
    for osc in range(oscArr[-1]):
        init = time[idx]
        ID0  = ID[idx]
        number = oscArr[idx]
        while(idx<len(ID) and number == oscArr[idx]):
            idx+=1
        MTlist.append({'ID0':ID0, 'IDf':ID[idx], 'IDm': (ID[idx]+ID0)/2, 'MT': time[idx]-init})
    return MTlist


########################################################################
############        PLOTTING FUNCTIONS   ###############################
########################################################################
  
def plotMTInd(resall):
    for subject, results in resall.iteritems():       
        for conf, data in results.iteritems():
            _plotMTInd(conf, data, subject)

def plotMTCombinedDouble(resall):
    for subject, results in resall.iteritems():
        opList = list()
        for conf, data in results.iteritems():
            opConf = getOpossite(conf)
            if opConf not in opList:
                opList.append(opConf)
            else:
                continue
            _plotMTCombinedDouble(conf, opConf, data, results[opConf],subject)            


def plotMTCombined(resall):
    for subject, results in resall.iteritems():       
        for conf, data in results.iteritems():
            _plotMTCombined(conf, data, subject)
            
def _plotMTCombinedDouble(conf,opConf, data, opData, subject='__'):
    mt, time, idS, id, p = data['all']
    _mt, _time, _idS, _id, _p = opData['all']
    
    time2 = numpy.linspace(time.min(),time.max(),30000)
    _time2 = numpy.linspace(_time.min(),_time.max(),30000)

    figname = os.path.join(grPath,subject+'_'+conf[0]+'-'+conf[1]+'_MTGraph_Combined'+grExt)
    figtitle = subject+' '+conf[0]+'-'+conf[1]+'  '+' Double plot'
    fig = pylab.figure()
    
    host = HostAxes(fig, [0.15, 0.1, 0.65, 0.8])
    par1 = ParasiteAxes(host, sharex=host)
    host.parasites.append(par1)

    host.set_ylabel("MT (ms)")
    host.set_xlabel("Time (ms)")

    host.axis["right"].set_visible(False)
    par1.axis["right"].set_visible(True)
    par1.set_ylabel("ID")

    par1.axis["right"].major_ticklabels.set_visible(True)
    par1.axis["right"].label.set_visible(True)

    fig.add_axes(host)
    
    sigmoidArr = sigmoid(p,time2)
    _sigmoidArr = sigmoid(_p,_time2)
    t2 = time2[numpy.where(abs(sigmoidArr - (sigmoidArr.min() + sigmoidArr.max())/2)< eps2)[0]]     
    _t2 = time2[numpy.where(abs(_sigmoidArr - (_sigmoidArr.min() + _sigmoidArr.max())/2)< eps2)[0]]
    t50 = t2
    _t50 = _t2
    if isinstance(t50, numpy.ndarray):
        t50 = t50.mean()
    if isinstance(_t50, numpy.ndarray):
        _t50 = _t50.mean()
    host.set_xlim(0, 90000)
    #host.set_ylim(mt.min(),mt.max())
    
    if conf[0] == '4':
        host.set_ylim(200,900)
        host.set_xlim(0, 70000)
    else:
        host.set_ylim(200,1600)
        host.set_xlim(0, 90000)

    host.scatter(time,mt,s=0.1,c='r',marker='x',linewidth=0.1,label="MT")
    host.scatter(_time,_mt,s=0.1,c='b',marker='x',linewidth=0.1,label="MT")
    host.plot(time2,sigmoid(p,time2),'-r',linewidth=1.5,label="Fitted MT")
    host.plot(_time2,sigmoid(_p,_time2),'-b',linewidth=1.5,label="Fitted MT")
    par1.scatter(time, id, s=1, c='r',marker='o',linewidth=0.5,label="ID") 
    par1.scatter(_time, _id,s=1,c='b',marker='o',linewidth=0.5,label="ID") 
    msg = "MT50-1 = %f; MT50-2 = %f" % (t50,_t50)
    host.text(  0.2 , 0.95, msg, fontsize = 8,\
                horizontalalignment='left', verticalalignment='center',\
                transform = host.transAxes)
    host.vlines(t50,mt.min(),mt.max())
    host.vlines(_t50,mt.min(),mt.max())
    if conf[0] == '4':
        host.set_ylim(200,900)
        host.set_xlim(0, 70000)
    else:
        host.set_ylim(200,1600)
        host.set_xlim(0, 90000)
    par1.set_ylim(min(int(conf[0]),int(conf[1])),max(int(conf[0]),int(conf[1])))
    #host.legend()
    host.set_title(figtitle)
    fig.savefig(figname)
    pylab.close() 
            
def _plotMTCombined(conf, data, subject='__'):
    mt, time, idS, id, p = data['all']
    time2 = numpy.linspace(time.min(),time.max(),30000)
    figname = os.path.join(grPath,subject+'_'+conf[0]+'-'+conf[1]+'_'+conf[2]+'_'+'_MTGraph_All'+grExt)
    figtitle = subject+' '+conf[0]+'-'+conf[1]+'  '+conf[2]+'Combined replications'
    fig = pylab.figure()
    
    host = HostAxes(fig, [0.15, 0.1, 0.65, 0.8])
    par1 = ParasiteAxes(host, sharex=host)
    host.parasites.append(par1)

    host.set_ylabel("MT (ms)")
    host.set_xlabel("Time (ms)")

    host.axis["right"].set_visible(False)
    par1.axis["right"].set_visible(True)
    par1.set_ylabel("ID")

    par1.axis["right"].major_ticklabels.set_visible(True)
    par1.axis["right"].label.set_visible(True)

    fig.add_axes(host)

    sigmoidArr = sigmoid(p,time2)
    t2 = time2[numpy.where(abs(sigmoidArr - (sigmoidArr.min() + sigmoidArr.max())/2) < eps2)[0]]
    #t50 = abs(t2-time2.max())
    t50 = t2
    if isinstance(t50, numpy.ndarray):
        t50 = t50.mean()

    #host.set_ylim(mt.min(),mt.max())
    if conf[0] == '4':
        host.set_ylim(200,900)
        host.set_xlim(0, 70000)
    else:
        host.set_ylim(200,1600)
        host.set_xlim(0, 90000)   
    host.scatter(time,mt,s=0.1,marker='x',linewidth=0.1,label="MT")
    host.plot(time2,sigmoid(p,time2),'-r',linewidth=1.5,label="Fitted MT")
    par1.scatter(time, id, s=1, c='r',label="ID", linewidth=0.5) 
    msg = "MT50 = %f" % t50
    host.text(  0.3 , 0.95, msg, fontsize = 8,\
                horizontalalignment='left', verticalalignment='center',\
                transform = host.transAxes)
    host.vlines(t50,mt.min(),mt.max())
    #host.set_ylim(mt.min(),mt.max())
    if conf[0] == '4':
        host.set_ylim(200,900)
        host.set_xlim(0, 70000)
    else:
        host.set_ylim(200,1600)
        host.set_xlim(0, 90000)
    
    par1.set_ylim(min(int(conf[0]),int(conf[1])),max(int(conf[0]),int(conf[1])))
    #host.legend()
    host.set_title(figtitle)
    fig.savefig(figname)
    pylab.close()  

def _plotMTInd(conf, data, subject='__'):
    for i , (mt, time, idS, id,p) in enumerate(data['ind']):
        time2 = numpy.linspace(time.min(),time.max(),30000)
        
        figname = os.path.join(grPath,subject+'_'+conf[0]+'-'+conf[1]+'_'+conf[2]+'_'+str(i)+'_MTGraph_OneTrial'+grExt)
        figtitle = subject+' '+conf[0]+'-'+conf[1]+'  '+conf[2]+' Replication no: '+str(i)
        fig = pylab.figure()
        

        host = HostAxes(fig, [0.15, 0.1, 0.65, 0.8])
        par1 = ParasiteAxes(host, sharex=host)
        host.parasites.append(par1)
        host.set_ylabel("MT (ms)")
        host.set_xlabel("Trial Time (ms)")
        par1.set_ylabel("ID")
        host.axis["right"].set_visible(False)
        par1.axis["right"].set_visible(True)
        par1.axis["right"].major_ticklabels.set_visible(True)
        par1.axis["right"].label.set_visible(True)

        fig.add_axes(host)
        sigmoidArr = sigmoid(p,time2)
        t50 = time2[numpy.where(abs(sigmoidArr - (sigmoidArr.min() + sigmoidArr.max())/2) < eps2)]
        if isinstance(t50, numpy.ndarray):
            t50 = t50.mean()
            
        if conf[0] == '4':
            host.set_ylim(200,900)
            host.set_xlim(0, 70000)
        else:
            host.set_ylim(200,1600)
            host.set_xlim(0, 90000)
            
        host.scatter(time,mt,s=0.1,marker='x',linewidth=0.5, label="MT")
        host.plot(time2,sigmoidArr,'-r',linewidth=1.5,label="Fitted MT")
        par1.scatter(time, id, s=1,c='g',marker='o',linewidth=0.7, label="ID") 
        msg = "MT50 = %f" % t50
        host.text(0.1 , 0.95, msg, fontsize = 6,\
            horizontalalignment='left', verticalalignment='center',\
            transform = host.transAxes)
        host.vlines(t50,mt.min(),mt.max())
        if conf[0] == '4':
            host.set_ylim(200,900)
            host.set_xlim(0, 70000)
        else:
            host.set_ylim(200,1600)
            host.set_xlim(0, 90000)
        par1.set_ylim(min(int(conf[0]),int(conf[1])),max(int(conf[0]),int(conf[1])))
        host.set_title(figtitle)
        print figname
        fig.savefig(figname)
        pylab.close()

########################################################################
############        DATA FITTING FUNCTIONS   ###########################
########################################################################

def filterOutliers(t, mt, idS, id):
    #First, get initial fit
    # [low asimptote, high asimptote, , steepness, x offset, steepness2]
    guess = numpy.array([mt[0],mt[-1],mt[0],0.0005,idS,0.5])
    p1  = fitSigmoid(t,mt,guess)
    sigmoidArr = sigmoid(p1[0],t)
    #Remove outliers
    in_ = numpy.zeros(t.shape,dtype=numpy.bool)    
    for i in range(t.size):
        env1 = abs(t - t[i]) < eps3
        #print env
        mtm = mt[env1].mean()
        mts = mt[env1].std()
        env2 =  abs(mt - mtm) > 2.5*mts
        in_ = in_ | (env1 & env2)
    out = numpy.logical_xor(in_,numpy.ones(t.shape,dtype=numpy.bool))
    #Compute a new fit    
    p2  = fitSigmoid(t[out],mt[out],p1[0])
    return mt[out],t[out],idS,id[out],p2[0]

def fitSigmoid(t,mt,p):
    # Fit the first set
    errfunc = lambda p, t, mt: (sigmoid(p, t) - mt) # Distance to the target function
    return leastsq(errfunc, p, args=(t, mt))

def sigmoid(p,x):
    return sigmoid6p(p,x)        
      
def sigmoid6p(p, x):
    """ Target function: Boltzmann form of the sigmoidal (6p)"""
    # [low, top, y0, groth rate, x offset, 0.5]
    return p[0] + (p[1]-p[0])/(1+p[2]*numpy.exp(p[3]*(p[4]-x)))**(1/p[5])

def sigmoid5p(p, x):
    """ Target function: Boltzmann form of the sigmoidal (6p)"""
    # [low, top, y0, groth rate, x offset, 0.5]
    return p[0] + (p[1]-p[0])/(1+p[2]*numpy.exp(p[3]*(p[4]-x)))
    
def sigmoid4p(p, x):
    """ Target function: variable slope sigmoid (4 parameters)"""
    # [low, top, MT50, slope]
    return p[0] + (p[1]-p[0])/(1+numpy.exp((p[2]-x)/p[3]))

def sigmoid3p(p, x):
    """ Target function: 3 parameters sigmoid"""
    # [low, top, MT50]
    return p[0] + (p[1]-p[0])/(1+numpy.exp(p[2]-x))

########################################################################
############        DATA LOADING FUNCTIONS   ###########################
########################################################################

def load_data():
    """
        Funcion que lee los datos de salida del experimento
    """
    results = dict()
    for subject in os.listdir(dataPath):
        dirname = os.path.join(dataPath, subject)
        files = os.listdir(dirname)
        files.sort()
        res = list()
        if subject == 'travis': 
            rows = 3
        else:
            rows=2
        for file in files:
            datafile = os.path.join(dirname,file)
            if os.path.getsize(datafile) < 400000:
                continue
            conf = getConfFromHeader(datafile)
            try:
                res.append((conf, numpy.loadtxt(datafile, dtype=dt,usecols=cols, skiprows=rows)))
            except IOError:
                print " Seems we have an empty file in %s \n" % datafile
        results[subject] = res[:]
    return results
    
def load_trial(trial):
    #Load data temporal series and configuration of trial
    conf = trial[0]

    v = trial[1]['vx']
    if v.size  <  5000: return None     
    b = v != 0
    v = v[b]
    x = trial[1]['x'][b]
    t = trial[1]['time'][b]
    id = trial[1]['ID'][b]
    err = trial[1]['errors'][b]

    #Check and skip invalid trials
    if  err[-1] >  15 or\
        id[-1]  != conf['IDf']:
        return None
        
    #Smooth data and get phase
    xs = smooth(x)
    vs = smooth(v)
    ph = getPhase(xs)

    #Get indexes with changing ID
    if conf['ID0'] < conf['IDf']:
        tup = (str(int(conf['ID0'])),str(int(conf['IDf'])),'up')
        #idx = numpy.where((id < conf['ID0']) & (id > conf['IDf']))[0]
    else:
        tup = (str(int(conf['IDf'])),str(int(conf['ID0'])),'down')
        #idx = numpy.where((id > conf['ID0']) & (id < conf['IDf']))[0]   
    #if idx.size == 0:
        #continue
    return {'conf':tup, 'x':xs, 'v':vs, 't': t, 'id':id,'ph':ph, 'errors':err}
    



########################################################################
############        AUXILIAR FUNCTIONS   ###############################
########################################################################
    
def derivate(func, time):
    return numpy.diff(func)/numpy.diff(time)
    
def getPhase(x):     
    return numpy.arctan2(x,hilbert(x))

def getPhase2(x,v):     
    return numpy.angle(numpy.complex(x,v))
    
def getIDShift(tr):
    return tr['t'][numpy.where(tr['id'] != tr['id'][0])[0][0]]
    
def getConfFromHeader(filename):
    f=open(filename, "r")
    return castHeader(dict(tuple(field.split('='))  for field in f.readline().split()))
    
def castHeader(header):
    for key, value in hFmt.iteritems():
        if value == 'i':
            header[key] = int(header[key])
        elif value == 'f':
            header[key] = float(header[key])
    return header

def getOpossite(conf):
    print conf
    if conf[:1] == 'up':
        r = 'down'
    else:
        r= 'up'
    #return (str(max(int(conf[0]),int(conf[1]))), str(min(int(conf[0]),int(conf[1]))), r)
    return conf[:-1]+(r,)
        
def smooth(x,window_len=11,window='hanning'):
    """smooth the data using a window with requested size.
    
    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal 
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.
    
    input:
        x: the input signal 
        window_len: the dimension of the smoothing window; should be an odd integer
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.

    output:
        the smoothed signal
        
    example:

    t=linspace(-2,2,0.1)
    x=sin(t)+randn(len(t))*0.1
    y=smooth(x)
    
    see also: 
    
    numpy.hanning, numpy.hamming, numpy.bartlett, numpy.blackman, numpy.convolve
    scipy.signal.lfilter
 
    TODO: the window parameter could be the window itself if an array instead of a string   
    """

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."
    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."
    if window_len<3:
        return x
    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'"


    s=numpy.r_[2*x[0]-x[window_len:1:-1],x,2*x[-1]-x[-1:-window_len:-1]]
    #print(len(s))
    if window == 'flat': #moving average
        w=ones(window_len,'d')
    else:
        w=eval('numpy.'+window+'(window_len)')

    y=numpy.convolve(w/w.sum(),s,mode='same')
    return y[window_len-1:-window_len+1]

if __name__ == '__main__':
    main()
