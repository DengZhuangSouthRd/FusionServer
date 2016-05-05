TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

LIBS += -lIce -lIceUtil -lpthread
LIBS += -L/usr/local/lib -lproj -lgdal


INCLUDEPATH += ./main/src/algorithm/fusion/gdal/include
INCLUDEPATH += ./main/src/algorithm/fusion/proj/include

SOURCES += \
    main/Server.cpp \
    main/src/utils/log.cpp \
    main/src/rpc/wisefuseRpc.cpp \
    main/src/threadpool/ThreadPool.cpp \
    main/src/threadpool/Task.cpp \
    main/src/threadpool/Mutex.cpp \
    main/src/algorithm/fusion/Tools.cpp \
    main/src/algorithm/fusion/PCAFusion.cpp \
    main/src/algorithm/fusion/PCA.cpp \
    main/src/algorithm/fusion/HSIFusion.cpp \
    main/src/algorithm/fusion/HSI.cpp \
    main/src/algorithm/fusion/HCSFusion.cpp \
    main/src/algorithm/fusion/HCS.cpp \
    main/src/algorithm/fusion/GramSchmidtFusion.cpp \
    main/src/algorithm/fusion/DWTFusion.cpp \
    main/src/algorithm/fusion/CurveletFusion.cpp \
    main/src/algorithm/fusion/BroveyFusion.cpp \
    main/src/algorithm/fusion/curvelet/ifdct_wrapping.cpp \
    main/src/algorithm/fusion/curvelet/fdct_wrapping.cpp \
    main/src/algorithm/fusion/curvelet/fdct_wrapping_param.cpp \
    main/src/algorithm/fusion/curvelet/fftw/wisdomio.c \
    main/src/algorithm/fusion/curvelet/fftw/wisdom.c \
    main/src/algorithm/fusion/curvelet/fftw/twiddle.c \
    main/src/algorithm/fusion/curvelet/fftw/timer.c \
    main/src/algorithm/fusion/curvelet/fftw/rader.c \
    main/src/algorithm/fusion/curvelet/fftw/putils.c \
    main/src/algorithm/fusion/curvelet/fftw/planner.c \
    main/src/algorithm/fusion/curvelet/fftw/generic.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_64.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_32.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_16.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_10.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_9.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_8.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_7.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_6.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_5.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_4.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_3.c \
    main/src/algorithm/fusion/curvelet/fftw/ftwi_2.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_64.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_32.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_16.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_10.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_9.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_8.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_7.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_6.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_5.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_4.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_3.c \
    main/src/algorithm/fusion/curvelet/fftw/ftw_2.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_64.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_32.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_16.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_15.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_14.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_13.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_12.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_11.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_10.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_9.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_8.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_7.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_6.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_5.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_4.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_3.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_2.c \
    main/src/algorithm/fusion/curvelet/fftw/fni_1.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_64.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_32.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_16.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_15.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_14.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_13.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_12.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_11.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_10.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_9.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_8.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_7.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_6.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_5.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_4.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_3.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_2.c \
    main/src/algorithm/fusion/curvelet/fftw/fn_1.c \
    main/src/algorithm/fusion/curvelet/fftw/fftwnd.c \
    main/src/algorithm/fusion/curvelet/fftw/fftwf77.c \
    main/src/algorithm/fusion/curvelet/fftw/executor.c \
    main/src/algorithm/fusion/curvelet/fftw/config.c \
    main/src/algorithm/fusion/curvelet/fftw/malloc.cpp \
    main/src/utils/utils.cpp \
    main/src/algorithm/fusion/Fusion.cpp \
    main/src/algorithm/fusion/PgInf.cpp \
    main/src/algorithm/fusion/GdalInf.cpp \
    main/src/utils/jsoncpp.cpp \
    main/src/algorithm/quality/src/Clarity.cpp \
    main/src/algorithm/quality/src/ContrastRatio.cpp \
    main/src/algorithm/quality/src/Entropy.cpp \
    main/src/algorithm/quality/src/Mean.cpp \
    main/src/algorithm/quality/src/SignaltoNoiseRatio.cpp \
    main/src/algorithm/quality/src/Striperesidual.cpp \
    main/src/algorithm/quality/utils/qualityutils.cpp \
    main/src/rpc/qualityjudgeRpc.cpp \
    main/imagefusion.cpp \
    main/imagequality.cpp \
    main/src/algorithm/fusion/utils/fusionutils.cpp

HEADERS += \
    main/Server.h \
    main/src/utils/log.h \
    main/src/rpc/wisefuseRpc.h \
    main/src/threadpool/ThreadPool.h \
    main/src/threadpool/Task.h \
    main/src/threadpool/Mutex.h \
    main/src/algorithm/fusion/WT.h \
    main/src/algorithm/fusion/Tools.h \
    main/src/algorithm/fusion/PgInf.h \
    main/src/algorithm/fusion/PCAFusion.h \
    main/src/algorithm/fusion/PCA.h \
    main/src/algorithm/fusion/HSIFusion.h \
    main/src/algorithm/fusion/HSI.h \
    main/src/algorithm/fusion/HCSFusion.h \
    main/src/algorithm/fusion/HCS.h \
    main/src/algorithm/fusion/GramSchmidtFusion.h \
    main/src/algorithm/fusion/GdalInf.h \
    main/src/algorithm/fusion/DWTFusion.h \
    main/src/algorithm/fusion/CurveletFusion.h \
    main/src/algorithm/fusion/BroveyFusion.h \
    main/src/algorithm/fusion/curvelet/offvec.hpp \
    main/src/algorithm/fusion/curvelet/offmat.hpp \
    main/src/algorithm/fusion/curvelet/numvec.hpp \
    main/src/algorithm/fusion/curvelet/nummat.hpp \
    main/src/algorithm/fusion/curvelet/fdct_wrapping.hpp \
    main/src/algorithm/fusion/curvelet/fdct_wrapping_inline.hpp \
    main/src/algorithm/fusion/curvelet/fdct_wrapping_inc.hpp \
    main/src/algorithm/fusion/curvelet/fftw/fftw.h \
    main/src/algorithm/fusion/curvelet/fftw/fftw-int.h \
    main/src/algorithm/fusion/curvelet/fftw/config.h \
    main/src/utils/utils.h \
    main/src/utils/json/json-forwards.h \
    main/src/utils/json/json.h \
    main/src/algorithm/quality/utils/qualityUtils.h \
    main/src/rpc/qualityjudgeRpc.h \
    main/imagefusion.h \
    main/imagequality.h \
    main/src/algorithm/fusion/utils/fusionutils.h

DISTFILES +=

