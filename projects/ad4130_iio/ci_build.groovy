def getBuildMatrix(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG', 'RTD_2WIRE_CONFIG', 'RTD_3WIRE_CONFIG', 'RTD_4WIRE_CONFIG', 'THERMISTOR_CONFIG', 'THERMOCOUPLE_CONFIG', 'LOADCELL_CONFIG', 'ECG_CONFIG', 'NOISE_TEST_CONFIG', 'POWER_TEST_CONFIG'],
			IIO_CLIENT: ['REMOTE', 'LOCAL']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG'],
			IIO_CLIENT: ['REMOTE']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip 'NUCLEO_L552ZE_Q and SDP_K1 platforms + IIO Local client'
		!((axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' || axis['PLATFORM_NAME'] == 'SDP_K1') && (axis['IIO_CLIENT'] == 'LOCAL')) &&
		// Skip 'NUCLEO_L552ZE_Q and DISCO_F769NI platforms + SDRAM'
		!((axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' || axis['PLATFORM_NAME'] == 'DISCO_F769NI') && (axis['SDRAM'] == 'USE_SDRAM')) &&
		// Skip 'NUCLEO_L552ZE_Q and DISCO_F769NI platforms + VCOM'
		!((axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' || axis['PLATFORM_NAME'] == 'DISCO_F769NI') && (axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT')) &&
		// Skip 'DISCO_F769NI + all custom configs except user config + IIO Local client'
		!((axis['PLATFORM_NAME'] == 'DISCO_F769NI') &&
		  (axis['ACTIVE_DEMO_MODE_CONFIG'] != 'USER_DEFAULT_CONFIG') &&
		  (axis['IIO_CLIENT'] == 'LOCAL')) &&
		// Skip 'SDP_k1 + all custom configs except user config + PhyCOM Port'
		!((axis['PLATFORM_NAME'] == 'SDP_K1') &&
		  (axis['ACTIVE_DEMO_MODE_CONFIG'] != 'USER_DEFAULT_CONFIG') &&
		  (axis['COM_TYPE'] == 'USE_PHY_COM_PORT')) &&
		// Skip 'SDP_k1 + all custom configs except user config + No SDRAM'
		!((axis['PLATFORM_NAME'] == 'SDP_K1') &&
		  (axis['ACTIVE_DEMO_MODE_CONFIG'] != 'USER_DEFAULT_CONFIG') &&
		  (axis['SDRAM'] == 'NO_SDRAM'))
	}

	return buildMatrix
}

def doBuild(Map config =[:], projectName) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}-${config.IIO_CLIENT}") {
		FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} platform and ${config.ACTIVE_DEVICE} device with ${config.COM_TYPE} UART type, ${config.ACTIVE_DEMO_MODE_CONFIG}, ${config.SDRAM} and ${config.IIO_CLIENT} IIO client"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} IIO_CLIENT=${config.IIO_CLIENT} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}-${config.IIO_CLIENT} NEW_CFLAGS+=-DPLATFORM_NAME=${config.PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-DACTIVE_DEMO_MODE_CONFIG=${config.ACTIVE_DEMO_MODE_CONFIG} NEW_CFLAGS+=-D${config.COM_TYPE} NEW_CFLAGS+=-D${config.SDRAM} NEW_CFLAGS+=-DIIO_CLIENT=${config.IIO_CLIENT}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}-${config.IIO_CLIENT}"
		sh "cd projects/${projectName} ; make reset LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def getTestMatrix(projectName) {
	Map testMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for testing all targets when merging to main or develop branches
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG'],
			IIO_CLIENT: ['REMOTE']
		]
	} else {
		// This map is for testing target that is always tested
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG'],
			IIO_CLIENT: ['REMOTE']
		]
	}

	List testMatrix = buildInfo.getMatrixCombination(testMap)
	
	return testMatrix
}

def doTest(Map config =[:], projectName) {
	node(label : "${hw.getAgentLabel(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVB-REF-' + config.ACTIVE_DEVICE)}") {
		checkout scm
		try {
			stage("Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}-${config.IIO_CLIENT}") {
				// Fetch the stashed files from build stage
				unstash "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}-${config.IIO_CLIENT}"

				echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"                       
				echo "Running on node: '${env.NODE_NAME}'"
				echo "Testing for ${config.PLATFORM_NAME} platform and ${config.ACTIVE_DEVICE} device with ${config.COM_TYPE} UART type, ${config.ACTIVE_DEMO_MODE_CONFIG}, ${config.SDRAM} and ${config.IIO_CLIENT} IIO client"
				echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
				
				platformInfo = hw.getPlatformInfo(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVB-REF-' + config.ACTIVE_DEVICE)
				echo "Programming MCU platform..."               
				bat "mbedflsh --version"
				// need to ignore return code from mbedflsh as it returns 1 when programming successful
				bat returnStatus: true , script: "mbedflsh --disk ${platformInfo["mountpoint"]} --file ${WORKSPACE}\\projects\\${projectName}\\build\\${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}-${config.IIO_CLIENT}.bin"
				bat "cd projects/${projectName}/tests & pytest --rootdir=${WORKSPACE}\\projects\\${projectName}\\tests -c pytest.ini --serialport ${platformInfo["serialport"]} --device_name ${config.ACTIVE_DEVICE} --platform_name ${config.PLATFORM_NAME} --serial_com_type ${config.COM_TYPE}"			
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