import xlrd
import xlwt
import re
from collections import namedtuple
    
sTotalSize = 0
sSubSize = 0

srcFile = input("Please input fileName : ")
file = xlrd.open_workbook( srcFile )

structFile = open("cvi_isp_mpi_def.h" , 'w')    #?
enumFile = open("isp_param_def.h" , 'w')
api2File = open("cvi_isp_block.c" , 'w')
initFile = open("isp_param_init.c" , 'w')

sheetName = file.sheets()

structList = list()
param = list()
lowVal = list()
highVal= list()
defVal = list()
autoVal = list()
regHighVal = list()
getApi = list()
setApi = list()
reg = list()
autoStruct = list()
manualStruct = list()
enumList = list()
excelLocate = dict()
normalSize = list()
manualSize = list()
paramInit = list()
regDefVal = list()
regLowVal = list()
tableParam = list()
structParamName = list()
manualParamName = list()

unitSize = 0
totalSize = 0
sheetNum = 0
paramName = ''
structName = ''

api2File.write( "\n" )

def arraySizeCheck( string , index ) :
    result = ''
    for i in range( index , len(string)) :
        if string[ i ].isdigit() :
            result += string[ i ]
    arraySize = int( result )
    print( "arraySize =" + str( arraySize ) )
    return arraySize

def paramInfoCheck( paramString ) :
    size = 1
    if paramString.count('[') :
        arraySize = arraySizeCheck( paramString , paramString.find('['))
        temp = paramString.split('[')
        paramName = temp[ 0 ]
    else :
        arraySize = 0
        paramName = paramString
    return paramName , arraySize    

def sizeCheck( lowValue , highValue ) :
    #print( "low :" , lowValue , highValue )
    for i in range( 32 ) :
        val = 1 << i
        if int(highValue) < val :
            break
    if lowValue < 0 :
        symbol = 1
    else :
        symbol = 0
    if i == 0 :
        return 0 , symbol , 1
    elif i <= 8 :
        return 8 , symbol , 1
    elif i <=16 :
        return 16 , symbol , 2
    else :
        return 32 , symbol , 4

def enumValSet( unitSize , shift , totalSize ) :
    num = ( ( unitSize ) << 16 ) | totalSize
    if unitSize <= 2 :
        totalSize += ( 1 << shift )
    else :     
        totalSize += ( 2 << shift )
    return num , totalSize

enumFile.write( "typedef enum {\n" )
for sheet in sheetName :
    name = sheet.name
    #if sheet.name != "DRC" :
    #    continue

    if name.find("Cali") != -1 or name.find("_rev") != -1:
        continue 
    cols = sheet.col_values(7)
    print( name.upper() , len( cols ) )
    
    structList.clear()
    normalSize.clear()
    param.clear()
    lowVal.clear()
    highVal.clear()
    defVal.clear()
    autoVal.clear()
    reg.clear()
    regHighVal.clear()
    regLowVal.clear()
    regDefVal.clear()
    autoStruct.clear()
    manualStruct.clear()
    enumList.clear()
    manualSize.clear()
    excelLocate.clear()
    paramInit.clear()
    setApi.clear()
    getApi.clear()
    tableParam.clear()
    structParamName.clear()
    manualParamName.clear()
    
    hasAuto = 0
    start = 0
    private = 0
    locate = 0
    skip = 0
    for i in range( 1 , len( cols ) ) :
        param.append(sheet.cell_value( i , 2 ))
        lowVal.append(sheet.cell_value( i , 3 ))
        highVal.append(sheet.cell_value( i , 4 ))
        defVal.append(sheet.cell_value( i , 5 ))
        autoVal.append(sheet.cell_value( i , 6 ))
        reg.append(sheet.cell_value( i , 7 ))
        regHighVal.append(sheet.cell_value( i , 10 ))
        regLowVal.append(sheet.cell_value( i , 9 ))
        regDefVal.append(sheet.cell_value( i , 11 ))
    
    structList.append( "typedef struct bmISP_" + name.upper() + "_ATTR_S\n" + "{\n" )
    autoStruct.append( "typedef struct bmISP_" + name.upper() + "_AUTO_ATTR_S\n" + "{\n")
    manualStruct.append( "typedef struct bmISP_" + name.upper() + "_MANUAL_ATTR_S\n" + "{\n")
    paramInit.append("int " + "isp_param_" + name.lower() + "_init( int ViPipe ) \n{\n")
    setAPI = "int BM_MPI_ISP_Set"+name+"Attr(int ViPipe, const ISP_"\
             +name.upper()+"_ATTR_S *pst"+name+"Attr)\n"
    getAPI = "int BM_MPI_ISP_Get"+name+"Attr(int ViPipe, ISP_"\
             +name.upper()+"_ATTR_S *pst"+name+"Attr)\n"
    structName = "pst"+name+"Attr->"
    setApi.append( setAPI + "{\n" )
    getApi.append( getAPI + "{\n" )
    print( len(reg) )
    count = 1
    print('name = ' , name )
    if name.count('ColorTone') or name.count('BlackLevel') :
        blockNum = 4
    else :
        blockNum = 1
    for i in range( len( param ) ) :
        print( i , lowVal[i] , highVal[i] , autoVal[i] , reg[ i ].find("[") , param[i] )
        if param[i] != '' :
            print("LowVal[i] , " , regLowVal[i] , regHighVal[i] )
            if lowVal[i] =="" or highVal[i] =="":
                size , symbol , unitSize = sizeCheck( regLowVal[i] , regHighVal[i] )
            else :    
                size , symbol , unitSize = sizeCheck( lowVal[i] , highVal[i] )
        else :
            continue 
        print( size , symbol )
        if size == 0 :
            sizeStr = 'bool '
        elif size == 8 :
            if symbol == 1 :
                sizeStr = 's8 '
            else :
                sizeStr = 'u8 '
        elif size == 16 :
            if symbol == 1 :
                sizeStr = 's16 '
            else :
                sizeStr = 'u16 '
        else :
            if symbol == 1 :
                sizeStr = 's32 '
            else :
                sizeStr = 'u32 '
        
        if autoVal[i] == 0 :
            excelLocate[ param[ i ] ] = i 
            if param[ i ].find("N.A") != -1 :
                param[ i ] = param[ i ].rstrip("(N.A)")
            print("    " + sizeStr + param[ i ] + ";\n")
            structParamName.append( param[ i ] )
            excelLocate[ param[ i ] ] = i
            structList.append( "    " + sizeStr + param[ i ] + ";\n" )
            normalSize.append( unitSize )
        elif autoVal[i] == 1.0 :
            hasAuto = 1
            if param[ i ].find("N.A") != -1 :
                param[ i ] = param[ i ].rstrip("(N.A)")
            manualParamName.append( param[ i ] )
            excelLocate[ param[ i ] ] = i
            manualSize.append( unitSize )
            manualStruct.append( "    " + sizeStr + param[ i ] + ";\n" )
            if param[ i ].find('[') != -1 :
                arraySize = arraySizeCheck( param[ i ] , param[ i ].find("[") )
                paramList = param[ i ].split('[')
                print("param[i] " , param[ i ] , arraySize , paramList )
                autoStruct.append(  "    " + sizeStr + paramList[ 0 ] + "[ISP_AUTO_ISO_STRENGTH_NUM][" + str(arraySize) +"];\n" )
            else :
                autoStruct.append(  "    " + sizeStr + param[ i ] + "[ISP_AUTO_ISO_STRENGTH_NUM];\n" )
                
    if hasAuto == 1 :
        structList.append( "    " + "ISP_OP_TYPE_E enOpType;\n" )
        structList.append( "    " + "ISP_" + name.upper() + "_MANUAL_ATTR_S stManual;\n" )
        structList.append( "    " + "ISP_" + name.upper() + "_AUTO_ATTR_S stAuto;")
        structParamName.append( "enOpType")
        structParamName.append( "stManual")
        structParamName.append( "stAuto")
        setAPI = "    int i = 0 ;\n"
        setApi.append( setAPI )
        getAPI = "    int i = 0 ;\n"
        getApi.append( getAPI )
        
    structList.append( "\n" + "} ISP_" + name.upper() + "_ATTR_S;\n\n")
    autoStruct.append( "} ISP_" + name.upper() + "_AUTO_ATTR_S ;\n\n")
    manualStruct.append( "}ISP_" + name.upper() + "_MANUAL_ATTR_S;\n\n" )
    print( "len : " , len( manualStruct ) , len( autoStruct ) , len( structList ))  
                    
    for i in range( 0 , len( structParamName ) ):
        paramName , arraySize = paramInfoCheck( structParamName[ i ] )
        if paramName == "stManual" :
            autoIdx = 0 
            print("stManual;stManual;stManual;stManual;")
            for i in range( 0 , len( manualParamName ) ) :
                manualName , arraySize = paramInfoCheck( manualParamName[ i ] )
                if arraySize > 256 :
                    tableParam.append( manualName )
                    continue 
                print("manualParam" , manualName , arraySize , autoIdx )
                if arraySize != 0 :
                    setAPI = "    for( i = 0 ; i < " + str( arraySize ) + " ; i++ ) {\n"
                    getAPI = "    for( i = 0 ; i < " + str( arraySize ) + " ; i++ ) {\n"
                    setApi.append( setAPI )
                    getApi.append( getAPI )
                    result = structName + paramName + "." + manualName + '[ i ] '
                    enum = "ISP_" + name.upper() + "_MANUAL_" +  manualName + "_"
                    setAPI = "        " + "isp_param_set( ViPipe , " + enum + "0 + i , " + result + " );\n"
                    getAPI = "        " + "isp_param_get( ViPipe , " + enum + "0 + i , (void *)&( " + result + " ));\n"
                    setApi.append( setAPI )
                    getApi.append( getAPI )
                    for j in range( arraySize ) :
                        enumResult = enum + str( j )
                        num , totalSize = enumValSet( manualSize[ autoIdx ] , 0 , totalSize )
                        enumList.append( "    " + enumResult + " = " + hex( num ) + ",\n" )
                    setApi.append( "    }\n" )
                    getApi.append( "    }\n" )
                else :
                    num , totalSize = enumValSet( manualSize[ autoIdx ] , 0 , totalSize )
                    result = structName + paramName + "." + manualName
                    enum = "ISP_" + name.upper() + "_MANUAL_" +  manualName.upper() 
                    setAPI = "    " + "isp_param_set( ViPipe , " + enum + " , " + result + " );\n"
                    getAPI = "    " + "isp_param_get( ViPipe , " + enum + " , (void *)&( " + result + " ));\n"
                    print("excelLocate[ manualParamName ] " , excelLocate[ manualParamName[ i ] ] , manualParamName[i] )
                    defaultValue = defVal[ excelLocate[ manualParamName[ i ] ] ]
                    paramInit.append( "    " + "isp_param_set( ViPipe , " + enum + " , " + str(int(defaultValue)) + " );\n")
                    setApi.append( setAPI )
                    getApi.append( getAPI )
                    enumList.append( "    " + enum + " = " + hex( num ) + ",\n" )
                autoIdx += 1                               
        elif paramName == "stAuto" :
            autoIdx = 0
            setAPI = "    for( i = 0 ; i < ISP_AUTO_ISO_STRENGTH_NUM ; i++ ) {\n"
            setApi.append( setAPI )
            getAPI = "    for( i = 0 ; i < ISP_AUTO_ISO_STRENGTH_NUM ; i++ ) {\n"
            getApi.append( getAPI )
            for i in range( 0 , len( manualParamName ) ) :
                autoName , arraySize = paramInfoCheck( manualParamName[ i ] )
                if arraySize > 0 :
                    result = structName + paramName + "." + autoName + "[ i ]"
                    enum = "ISP_" + name.upper() + "_AUTO_" + autoName.upper() 
                    
                    setAPI = "        " + "isp_param_set( ViPipe , " + enum + "_0 + i " + "," + result + " );\n"
                    getAPI = "        " + "isp_param_get( ViPipe , " + enum + "_0 + i " + ", (void *)&(" + result + "));\n"
                    for k in range( 16 ) :
                        num = ( ( manualSize[ autoIdx ] * arraySize ) << 16 ) | totalSize
                        if manualSize[ autoIdx ] <= 2 :
                            totalSize += 1 * arraySize
                        else :     
                            totalSize += 2 * arraySize
                        enumList.append( "    " + enum + "_" + str(k) + " = " + hex( num ) + ",\n")
                else :
                    result = structName + paramName + "." + autoName + "[ i ]"
                    enum = "ISP_" + name.upper() + "_AUTO_" +  autoName.upper() 
                    num , totalSize = enumValSet( manualSize[ autoIdx ] , 4 , totalSize )
                    if manualSize[ autoIdx ] <= 2 :
                        setAPI = "        " + "isp_param_set( ViPipe , " + enum + "+ i " + "," + result + " );\n"
                        getAPI = "        " + "isp_param_get( ViPipe , " + enum + "+ i " + ", (void *)&(" + result + "));\n"
                    else :
                        setAPI = "        " + "isp_param_set( ViPipe , " + enum + "+ i * " + str( manualSize[ autoIdx ] >> 1 )\
                            + " ," + result + " );\n"
                        getAPI = "        " + "isp_param_get( ViPipe , " + enum + "+ i * " + str(manualSize[ autoIdx ] >> 1 )\
                            + " , (void *)&(" + result + " ) );\n"
                    enumList.append( "    " + enum + " = " + hex( num ) + ",\n")
                setApi.append( setAPI )
                getApi.append( getAPI )
                autoIdx += 1
            setAPI = "    }\n"
            setApi.append( setAPI )
            getAPI = "    }\n"
            getApi.append( getAPI )
        else :
            if arraySize > 256 :
                tableParam.append( paramName )
                continue 
            enum = "ISP_" + name.upper() + "_" +  paramName.upper()
            result = structName + paramName
            setAPI = "    " + "isp_param_set( ViPipe , " + enum + " , " + result + " );\n"
            setApi.append( setAPI )
            getAPI = "    " + "isp_param_get( ViPipe , " + enum + " , (void *)&(" + result + " ));\n"
            getApi.append( getAPI )
            if paramName == "enOpType" :
                num = ( 4 << 16 ) | totalSize
                totalSize += 2
                enumList.append( "    " + enum + " = " + hex( num ) + ",\n")
                paramInit.append( "    " + "isp_param_set( ViPipe , " + enum + " , OP_TYPE_MANUAL );\n")
            else :
                print( paramName  )
                defaultValue = defVal[ excelLocate[ structParamName[ i ] ] ]
                if structParamName[ i ].find('[') :
                    defaultValue = 0
                for j in range( arraySize ) :
                    enumResult = enum + str( j )
                    num , totalSize = enumValSet( normalSize[ i ] , 0 , totalSize )
                    enumList.append( "    " + enumResult + " = " + hex( num ) + ",\n" )    
                print( "nonStruct " , enum , paramName, defaultValue )
                paramInit.append( "    " + "isp_param_set( ViPipe , " + enum + " , " + str(int(defaultValue)) + " );\n")
                num , totalSize = enumValSet( normalSize[ i ] , 0 , totalSize )
                enumList.append( "    " + enum + " = " + hex( num ) + ",\n")
        paramName = ""
    for i in range( len( tableParam ) )  :
        print("tableParam[ i ]. ", tableParam )
        enum = "ISP_" + tableParam[ i ].upper()
        num = ( 511 << 16 ) | totalSize
        totalSize += 8 
        enumList.append( "    " + enum + " = " + hex( num ) + ",\n")
        
    totalSize = ( totalSize + 63 ) >> 6 << 6
    setApi.append( "}\n\n" )
    getApi.append( "}\n\n" )
    paramInit.append("}\n\n")    
    for i in range( len( manualStruct )) :
        structFile.write( manualStruct[i] )
    for i in range( len( autoStruct )) :
        structFile.write( autoStruct[i] )
    for i in range( len( structList )) :
        structFile.write( structList[i] )
    for i in range( len( setApi )) :
        api2File.write( setApi[i] )
    for i in range( len( getApi )) :
        api2File.write( getApi[i] )    
    for i in range( len( enumList )) :
        enumFile.write( enumList[i] )
    for i in range( len( paramInit )) :
        initFile.write( paramInit[i] )    
    sheetNum += 1
    if sheetNum == 15 :
        break

structFile.close()
api2File.close()
enumFile.close()
initFile.close()
