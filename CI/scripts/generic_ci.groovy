def findProjectChanged() {
	lastCommit_parentHashs_withcommand = bat (returnStdout: true, script: 'git show -s --pretty=%%p HEAD').trim()
	lastCommit_parentHashs = removeFirstLine(lastCommit_parentHashs_withcommand)
	String[] seperate_parentHashs = lastCommit_parentHashs.split(' ')
	int lastCommit_noOfParents = seperate_parentHashs.size()
	if ( lastCommit_noOfParents >= 2) {
		repoChanges_withcommand = bat(returnStdout: true, script: 'git diff-tree --no-commit-id --name-only -r HEAD HEAD^^2').trim()
		repoChanges = removeFirstLine(repoChanges_withcommand)
	} else { 
		repoChanges_withcommand = bat(returnStdout: true, script: 'git diff-tree --no-commit-id --name-only -r HEAD').trim()
		repoChanges = removeFirstLine(repoChanges_withcommand)
	}

	// Detect changes in each directory of repository

	if (repoChanges.contains("ad7689_iio-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad7689_iio-application\\ad7689_ci.groovy'
		def buildStatus = groovyScript.ad7689Build()
		if (buildStatus != "Failed") {
			groovyScript.ad7689Test()
		}
	}

	if (repoChanges.contains("ad4130_iio-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad4130_iio-application\\ad4130_ci.groovy'
		def buildStatus = groovyScript.ad4130Build()
		if (buildStatus != "Failed") {
			groovyScript.ad4130Test()
		}
	}
	
	if (repoChanges.contains("ad717x_iio-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad717x_iio-application\\ad717x_ci.groovy'
		def buildStatus = groovyScript.ad717xBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad717xTest()
		}
	}

	if (repoChanges.contains("ad7124_temperature-measure-example") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad7124_temperature-measure-example\\ad7124_temp_measure_ci.groovy'
		def buildStatus = groovyScript.ad7124TempMeasureBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad7124TempMeasureTest()
		}
	}	

	if (repoChanges.contains("ad4696_iio-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad4696_iio-application\\ad4696_ci.groovy'
		def buildStatus = groovyScript.ad4696Build()
		if (buildStatus != "Failed") {
			groovyScript.ad4696Test()
		}
	}

	if (repoChanges.contains("ad717x_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad717x_console-application\\ad717x_console_ci.groovy'
		def buildStatus = groovyScript.ad717xConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad717xConsoleTest()
		}
	}
	
	if (repoChanges.contains("ad7124_console_application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad7124_console_application\\ad7124_console_ci.groovy'
		def buildStatus = groovyScript.ad7124ConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad7124ConsoleTest()
		}
	}

	if (repoChanges.contains("ad7606_iio-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad7606_iio-application\\ad7606_ci.groovy'
		def buildStatus = groovyScript.ad7606Build()
		if (buildStatus != "Failed") {
			groovyScript.ad7606Test()
		}
	}

	if (repoChanges.contains("nanodac_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'nanodac_console-application\\nanodac_console_ci.groovy'
		def buildStatus = groovyScript.nanodacConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.nanodacConsoleTest()
		}
	}
	
	if (repoChanges.contains("ltc268x_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ltc268x_console-application\\ltc268x_console_ci.groovy'
		def buildStatus = groovyScript.ltc268xBuild()
		// TODO : As the test setup is not available at remote machine,
		//        so no call is made to the test fuction.
	}

	if (repoChanges.contains("ad5770r_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad5770r_console-application\\ad5770r_console_ci.groovy'
		def buildStatus = groovyScript.ad5770rConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad5770rConsoleTest()
		}
	}

	if (repoChanges.contains("ad590_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad590_console-application\\ad590_console_ci.groovy'
		def buildStatus = groovyScript.ad590ConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad590ConsoleTest()
		}
	}

	if (repoChanges.contains("adt7420_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'adt7420_console-application\\adt7420_console_ci.groovy'
		def buildStatus = groovyScript.adt7420ConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.adt7420ConsoleTest()
		}
	}
	
	if (repoChanges.contains("ad5933_console-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad5933_console-application\\ad5933_console_ci.groovy'
		def buildStatus = groovyScript.ad5933ConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad5933ConsoleTest()
		}
	}

	if (repoChanges.contains("ad77681_iio-application") || repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad77681_iio-application\\ad77681_ci.groovy'
		def buildStatus = groovyScript.ad77681Build()
		if (buildStatus != "Failed") {
			groovyScript.ad77681Test()
		}
	}
	
	if (repoChanges.contains("ad559xr_console-application")|| repoChanges.contains("tools") || repoChanges.contains("CI") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
		groovyScript = load 'ad559xr_console-application\\ad559xr_console_ci.groovy'
		def buildStatus = groovyScript.ad559xrConsoleBuild()
		if (buildStatus != "Failed") {
			groovyScript.ad559xrConsoleTest()
		}
	}
}

// This function used to generate matrix combinations
@NonCPS
List getMatrixCombination(Map matrix =[:]) {
    List matrixAxes = []
    matrix.each { key, arguments ->
        List tempList = []
        arguments.each { argument ->
            tempList.add((key): argument)
        }
        matrixAxes.add(tempList)
    }
    matrixAxes.combinations().collect{it.sum()}
}

//This is used to remove first line of bat output as in first line of bat output is command we supplied to bat only
String removeFirstLine(String input_string) {
	String[] splitString = input_string.split('\n')
	String returnedString =""
	for (int i=1;i<splitString.size();i++) {
		returnedString = returnedString.concat(splitString[i])
		if(i != splitString.size()-1) {
			returnedString = returnedString.concat("\n")
		}
	}
	return returnedString
}
return this;
