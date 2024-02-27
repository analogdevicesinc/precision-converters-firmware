def getBuildMatrix(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD5780', 'DEV_AD5781', 'DEV_AD5790', 'DEV_AD5791', 'DEV_AD5760'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			DATA_STREAM_MODE: ['CYCLIC_STREAM'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DAC_REF: ['INT_REF_M10V_TO_10V', 'INT_REF_0V_TO_10V']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD5780'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			DATA_STREAM_MODE: ['CYCLIC_STREAM'],
			SDRAM: ['NO_SDRAM'],
			DAC_REF: ['INT_REF_M10V_TO_10V']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip 'all platforms except SDP-K1 + SDRAM and + VCOM and + devices other than ad5780'
		!(axis['PLATFORM_NAME'] != 'SDP_K1' && axis['SDRAM'] == 'USE_SDRAM') &&
		!(axis['PLATFORM_NAME'] != 'SDP_K1' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] != 'SDP_K1' && axis['ACTIVE_DEVICE'] != 'DEV_AD5780') &&
		// Skip combinations with devices ad5781, ad5791 with 0 to 10V reference option
		!(axis['ACTIVE_DEVICE']	== 'DEV_AD5781' && axis['DAC_REF'] == 'INT_REF_0V_TO_10V') &&
		!(axis['ACTIVE_DEVICE']	== 'DEV_AD5791' && axis['DAC_REF'] == 'INT_REF_0V_TO_10V')
	}

	return buildMatrix
}

def doBuild(Map config =[:], projectName) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.DATA_STREAM_MODE}-${config.SDRAM}-${config.DAC_REF}") {
		FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} Platform and ${config.ACTIVE_DEVICE} Device with ${config.COM_TYPE} UART type with ${config.DAC_REF} in ${config.DATA_STREAM_MODE} and ${config.SDRAM}"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.DATA_STREAM_MODE}-${config.SDRAM}-${config.DAC_REF} NEW_CFLAGS+=-DPLATFORM_NAME=${config.PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-DDATA_STREAM_MODE=${config.DATA_STREAM_MODE} NEW_CFLAGS+=-D${config.ACTIVE_DEVICE} NEW_CFLAGS+=-D${config.COM_TYPE} NEW_CFLAGS+=-D${config.SDRAM} NEW_CFLAGS+=-D${config.DAC_REF}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.DATA_STREAM_MODE}-${config.SDRAM}-${config.DAC_REF}"
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