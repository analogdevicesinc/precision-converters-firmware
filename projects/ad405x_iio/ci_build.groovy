def getBuildMatrix(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD4052', 'DEV_AD4050'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			APP_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE', 'WINDOWED_DATA_CAPTURE'],
		    ADC_CAPTURE_MODE: ['SAMPLE_MODE', 'BURST_AVERAGING_MODE']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: [ 'DEV_AD4052', 'DEV_AD4050'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['NO_SDRAM'],
			APP_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE'],
		    ADC_CAPTURE_MODE: ['SAMPLE_MODE']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip combinations other than SDP-K1/USE_SDRAM
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['SDRAM'] == 'USE_SDRAM') &&
    	!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['SDRAM'] == 'USE_SDRAM') &&

		// Skip combinations other than SDP-K1/USE_VIRTUAL_COM_PORT
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		
		// Skip combinations other than SDP-K1/CONTINUOUS_DATA_CAPTURE
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['APP_CAPTURE_MODE'] == 'WINDOWED_DATA_CAPTURE') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['APP_CAPTURE_MODE'] == 'WINDOWED_DATA_CAPTURE') &&
		
		// Skip combinations other than SDP-K1/SAMPLE_MODE
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ADC_CAPTURE_MODE'] == 'BURST_AVERAGING_MODE') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ADC_CAPTURE_MODE'] == 'BURST_AVERAGING_MODE')
	}
	
	return buildMatrix
}

def doBuild(Map config =[:], projectName) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.APP_CAPTURE_MODE}-${config.ADC_CAPTURE_MODE}") {
		FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} platform and ${config.ACTIVE_DEVICE} device with ${config.COM_TYPE} UART type, application in ${config.APP_CAPTURE_MODE}, ADC in ${config.ADC_CAPTURE_MODE} with ${config.SDRAM}"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.APP_CAPTURE_MODE}-${config.ADC_CAPTURE_MODE} NEW_CFLAGS+=-DPLATFORM_NAME=${config.PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-DADC_CAPTURE_MODE=${config.ADC_CAPTURE_MODE} NEW_CFLAGS+=-DAPP_CAPTURE_MODE=${config.APP_CAPTURE_MODE} NEW_CFLAGS+=-D${config.ACTIVE_DEVICE} NEW_CFLAGS+=-D${config.COM_TYPE} NEW_CFLAGS+=-D${config.SDRAM}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.APP_CAPTURE_MODE}-${config.ADC_CAPTURE_MODE}"
		sh "cd projects/${projectName} ; make reset LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME}"
	}
	
}

def getTestMatrix(projectName) {
	// TODO: Add the test sequence once CI test setup is up
	List testMatrix = []
	return testMatrix
}

def doTest(Map config =[:], projectName) {
	// TODO: Add the test sequence once CI test setup is up
}

return this;