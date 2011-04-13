VERSION=
ARCH=
PREFIX=
BUILD=


DEPS="ipp mkl opencv libz libfreenect libmesasr libjpeg unicap xine qt opengl glx opensurf gtest"

for T in $DEPS ; do 
    eval "path_$T=_" ; 
done

for T in "$@" ; do
    if [ ! -e "$T" ] ; then 
        echo "unable to find configuration file: $T" ;
        exit -1 ;
    fi
    
    while read line ; do
        if [[ ! "$line" =~ ^[\ \t]*\#.* ]] ; then      # comments
            if [ "$line" != "" ] ; then                   # empty lines
                if [[ ! "$line" =~ ^[\ \t]\$ ]] ; then # whitespace lines
                    T1=$(echo $line | cut -d ' ' -f 1 | tr '[:upper:]' '[:lower:]')
                    TR=$(echo $line | cut -d ' ' -s -f 2-)
                    #echo " processing $line (T1: $T1)"
                    case "$T1" in
                        "version") VERSION="$TR" ;;
                        "prefix") PREFIX="$TR" ;;
                        "build") BUILD="$TR" ;;
                        "arch") ARCH="$TR" ;;
                        *) eval "path_$T1=$TR" ;;
                    esac
                fi
            fi
        fi
    done < "$T"
done



if [ -z $VERSION ] ; then 
    VERSION=release
fi
if [ -z $BUILD ] ; then 
    BUILD=build/$VERSION
fi
if [ -z $ARCH ] ; then 
    ARCH=false
fi
if [ -z $PREFIX ] ; then 
    PREFIX=/vol/nivision
fi

for D in $DEPS ; do
    P=$(eval "echo path_$D")
    if [ -z $P ] ; then
        eval "path_$D=/usr"
    fi
done

CMD="cmake -D ICL_XDEP_ALL_ON:BOOL=FALSE -D ICL_VAR_INSTALL_PREFIX:STRING=$PREFIX "
CMD="$CMD -D ICL_VAR_DEBUG_MODE:BOOL=$(echo $VERSION | tr '[:lower:]' '[:upper:]')"
CMD="$CMD -D ICL_VAR_ARCHITECTURE_DEPENDENT_BUILD_ON:BOOL=$(echo $ARCH | tr '[:lower:]' '[:upper:]')"

echo "extracting configuration"
echo "-----------------------------------------------------"
for D in $DEPS ; do
    U=$(echo $D | tr '[:lower:]' '[:upper:]')
    P=$(echo $(eval echo \$path_$D))
    echo $D | awk '{printf "%-14s",$1}'
    if [ "$P" = "_" ] ; then
        CMD="$CMD -D ICL_XDEP_${U}_ON:BOOL=FALSE -D ICL_XDEP_${U}_PATH:STRING=/unexistent-directory"
        echo "off"
    elif [ "$P" = "" ] ; then
        CMD="$CMD -D ICL_XDEP_${U}_ON:BOOL=TRUE -D ICL_XDEP_${U}_PATH:STRING=/usr"
        echo "/usr (default)"
    else
        CMD="$CMD -D ICL_XDEP_${U}_ON:BOOL=TRUE -D ICL_XDEP_${U}_PATH:STRING=$P"
        echo "$P"
    fi
done
echo "-----------------------------------------------------"
echo "ICL version:             $(./VERSION.sh)"
echo "version (debug/release): $VERSION"
echo "build directory:         $BUILD"
echo "arch dependent build:    $ARCH"
echo "installation prefix:     $PREFIX"
echo "-----------------------------------------------------"



if [ ! -e $BUILD ] ; then
    echo creating build directory $BUILD ;
    mkdir -p $BUILD ;
fi

PREVDIR=$PWD
cd $BUILD

CMD="$CMD $PREVDIR"

echo "CMAKE command is"
echo "-----------------------------------------------------"
echo $CMD
echo "-----------------------------------------------------"
#$CMD