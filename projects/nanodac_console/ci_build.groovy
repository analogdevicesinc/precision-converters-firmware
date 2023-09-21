def getBuildMatrix(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			EVB_INTERFACE: ['SDP_120', 'ARDUINO'],
			ACTIVE_DEVICE: ['DEV_AD5686R', 'DEV_AD5676R', 'DEV_AD5683R', 'DEV_AD5677R']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			EVB_INTERFACE: ['SDP_120', 'ARDUINO'],
			ACTIVE_DEVICE: ['DEV_AD5686R']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip 'all platforms except SDP-K1 + SDP_120 EVB interface'
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['EVB_INTERFACE'] == 'SDP_120') &&
    	!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['EVB_INTERFACE'] == 'SDP_120') &&

		// Skip 'all platforms except SDP-K1 + all devices except DEV_AD5686R'
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD5676R') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD5683R') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD5677R') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD5676R') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD5683R') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD5677R')
	}

	return buildMatrix
}

def doBuild(Map config =[:], projectName) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.EVB_INTERFACE}") {
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} and ${config.ACTIVE_DEVICE} with ${config.EVB_INTERFACE} interface"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.EVB_INTERFACE} NEW_CFLAGS+=-D${config.ACTIVE_DEVICE} NEW_CFLAGS+=-D${config.EVB_INTERFACE}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.EVB_INTERFACE}"
		sh "cd projects/${projectName} ; make reset LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def getTestMatrix(projectName) {
	Map testMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for testing all targets when merging to main or develop branches
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			EVB_INTERFACE: ['SDP_120'],
			ACTIVE_DEVICE: ['DEV_AD5686R']
		]
	} else {
		// This map is for testing target that is always tested
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			EVB_INTERFACE: ['SDP_120'],
			ACTIVE_DEVICE: ['DEV_AD5686R']
		]
	}

	List testMatrix = buildInfo.getMatrixCombination(testMap)
	
	return testMatrix
}

def doTest(Map config =[:], projectName) {
	node(label : "${hw.getAgentLabel(platform_name: config.PLATFORM_NAME, evb_ref_id: "EVAL-AD5686RSDZ")}") {
		checkout scm
		try {
			stage("Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.EVB_INTERFACE}") {
				// Fetch the stashed files from build stage
				unstash "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.EVB_INTERFACE}"

				echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"                       
				echo "Running on node: '${env.NODE_NAME}'"
				echo "Testing for ${config.PLATFORM_NAME} and ${config.ACTIVE_DEVICE} with ${config.EVB_INTERFACE} interface"
				echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
				
				platformInfo = hw.getPlatformInfo(platform_name: config.PLATFORM_NAME, evb_ref_id: "EVAL-AD5686RSDZ")
				echo "Programming MCU platform..."               
				bat "mbedflsh --version"
				// need to ignore return code from mbedflsh as it returns 1 when programming successful
				bat returnStatus: true , script: "mbedflsh --disk ${platformInfo["mountpoint"]} --file ${WORKSPACE}\\projects\\${projectName}\\build\\${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.EVB_INTERFACE}.bin"               
				bat "cd projects/${projectName}/tests & pytest --rootdir=${WORKSPACE}\\projects\\${projectName}\\tests -c pytest.ini --serialnumber ${platformInfo["serialnumber"]}  --serialport ${platformInfo["serialport"]} --mountpoint ${platformInfo["mountpoint"]}"			
				archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/tests/output/*.csv"
				junit allowEmptyResults:true, testResults: "projects/${projectName}/tests/output/*.xml"
				deleteDir()
			}
		}
		catch (Exception ex) {
			deleteDir()
		}
	}
}

return this;