def getBuildMatrix(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			HW_CARRIER: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_LTC2672_12', 'DEV_LTC2672_16'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			HW_CARRIER: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_LTC2672_12', 'DEV_LTC2672_16'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['NO_SDRAM']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip combinations with USE_SDRAM for hw carriers other than SDP_K1 and
		!(axis['HW_CARRIER'] != 'SDP_K1' && axis['SDRAM'] == 'USE_SDRAM') &&
        // Skip combinations with VCOM_PORT for hw carriers other than SDP_K1
		!(axis['HW_CARRIER'] != 'SDP_K1' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		// Skip 'all platforms except SDP-K1 + all devices except LTC2672_16'
		!(axis['HW_CARRIER'] != 'SDP_K1' && axis['ACTIVE_DEVICE'] != 'DEV_LTC2672_16')
	}

	return buildMatrix
}

def doBuild(Map config =[:], projectName) {
	stage("Build-${config.HW_CARRIER}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}") {
		FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.HW_CARRIER} Platform and ${config.ACTIVE_DEVICE} Device with ${config.COM_TYPE} UART type and ${config.SDRAM}"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.HW_CARRIER} BINARY_FILE_NAME=${config.HW_CARRIER}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM} NEW_CFLAGS+=-DPLATFORM_NAME=${config.HW_CARRIER} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-D${config.ACTIVE_DEVICE} NEW_CFLAGS+=-D${config.COM_TYPE} NEW_CFLAGS+=-D${config.SDRAM}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.HW_CARRIER}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}"
		sh "cd projects/${projectName} ; make reset LINK_SRCS=n TARGET_BOARD=${config.HW_CARRIER}"
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
