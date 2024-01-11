def getBuildMatrix(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD7770'],
			DATA_TRANSFER_PORT: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DATA_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE', 'BURST_DATA_CAPTURE'],
			INTERFACE_MODE : ['SPI_MODE']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7770'],
			DATA_TRANSFER_PORT: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DATA_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE'],
			INTERFACE_MODE : ['SPI_MODE']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['SDRAM'] == 'USE_SDRAM') &&
    	!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['SDRAM'] == 'USE_SDRAM') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['DATA_TRANSFER_PORT'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['DATA_TRANSFER_PORT'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['DATA_CAPTURE_MODE'] == 'BURST_DATA_CAPTURE') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['DATA_CAPTURE_MODE'] == 'BURST_DATA_CAPTURE')
	}

	return buildMatrix
}

def doBuild(Map config =[:], projectName) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE}") {
		FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} Platform and ${config.ACTIVE_DEVICE} Device with ${config.DATA_TRANSFER_PORT} UART type and ${config.SDRAM} in ${config.DATA_CAPTURE_MODE} and ${config.INTERFACE_MODE}"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE} NEW_CFLAGS+=-DPLATFORM_NAME=${config.PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-DDATA_CAPTURE_MODE=${config.DATA_CAPTURE_MODE} NEW_CFLAGS+=-DINTERFACE_MODE=${config.INTERFACE_MODE} NEW_CFLAGS+=-D${config.ACTIVE_DEVICE} NEW_CFLAGS+=-D${config.DATA_TRANSFER_PORT} NEW_CFLAGS+=-D${config.SDRAM}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE}"
		sh "cd projects/${projectName} ; make reset LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def getTestMatrix(projectName) {
	Map testMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for testing all targets when merging to main or develop branches
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7770'],
			DATA_TRANSFER_PORT: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DATA_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE'],
			INTERFACE_MODE : ['SPI_MODE']
		]
	} else {
		// This map is for testing target that is always tested
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7770'],
			DATA_TRANSFER_PORT: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DATA_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE'],
			INTERFACE_MODE : ['SPI_MODE']
		]
	}

	List testMatrix = buildInfo.getMatrixCombination(testMap)

	return testMatrix
}

def doTest(Map config =[:], projectName) {
	node(label : "${hw.getAgentLabel(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVAL-AD777x-ARDZ')}") {
		checkout scm
		try {
			stage("Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE}") {
				// Fetch the stashed files from build stage
				unstash "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE}"

				echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"                       
				echo "Running on node: '${env.NODE_NAME}'"
				echo "Testing for ${config.PLATFORM_NAME} Platform and ${config.ACTIVE_DEVICE} Device with ${config.DATA_TRANSFER_PORT} UART type and ${config.SDRAM} in ${config.DATA_CAPTURE_MODE} and ${config.INTERFACE_MODE}"
				echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
				
				platformInfo = hw.getPlatformInfo(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVAL-AD777x-ARDZ')
				echo "Programming MCU platform..."               
				bat "mbedflsh --version"
				// need to ignore return code from mbedflsh as it returns 1 when programming successful
				bat returnStatus: true , script: "mbedflsh --disk ${platformInfo["mountpoint"]} --file ${WORKSPACE}\\projects\\${projectName}\\build\\${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE}.bin"               
				bat "cd projects/${projectName}/tests & pytest --rootdir=${WORKSPACE}\\projects\\${projectName}\\tests -c pytest.ini --serialport ${platformInfo["serialport"]} --device_name ${config.ACTIVE_DEVICE} --platform_name ${config.PLATFORM_NAME} --serial_com_type ${config.DATA_TRANSFER_PORT}"
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