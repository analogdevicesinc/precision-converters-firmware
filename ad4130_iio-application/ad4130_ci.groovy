// Project directory name
projectName = "ad4130_iio-application"

// This variable holds the build status
buildStatusInfo = "Success"

def ad4130Build() {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG', 'RTD_2WIRE_CONFIG', 'RTD_3WIRE_CONFIG', 'RTD_4WIRE_CONFIG', 'THERMISTOR_CONFIG', 'THERMOCOUPLE_CONFIG', 'LOADCELL_CONFIG', 'ECG_CONFIG', 'NOISE_TEST_CONFIG', 'POWER_TEST_CONFIG']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = general.getMatrixCombination(buildMap).findAll { axis ->
		// Skip 'all platforms except SDP-K1 + SDRAM and No SDRAM + all custom configs except user config + VCOM'
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['SDRAM'] == 'USE_SDRAM') &&
    	!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['SDRAM'] == 'USE_SDRAM') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_2WIRE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_3WIRE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_4WIRE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMISTOR_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMOCOUPLE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'LOADCELL_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'ECG_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'NOISE_TEST_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'POWER_TEST_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_2WIRE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_3WIRE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_4WIRE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMISTOR_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMOCOUPLE_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'LOADCELL_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'ECG_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'NOISE_TEST_CONFIG') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'POWER_TEST_CONFIG') &&

		// Skip 'SDP-K1 + SDRAM + all custom configs except user config + Phy COM'
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_2WIRE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_3WIRE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_4WIRE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMISTOR_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMOCOUPLE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'LOADCELL_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'ECG_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'NOISE_TEST_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'POWER_TEST_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&

		// Skip 'SDP-K1 + SDRAM + all custom configs except user config + VCOM'
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_2WIRE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_3WIRE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_4WIRE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMISTOR_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMOCOUPLE_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'LOADCELL_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'ECG_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'NOISE_TEST_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'POWER_TEST_CONFIG' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&

		// Skip 'SDP-K1 + No SDRAM + all custom configs except user config + Phy COM'
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_2WIRE_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_3WIRE_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'RTD_4WIRE_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMISTOR_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'THERMOCOUPLE_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'LOADCELL_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'ECG_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'NOISE_TEST_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEMO_MODE_CONFIG'] == 'POWER_TEST_CONFIG' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT')
	}

	def buildType = buildInfo.getBuildType()
	if (buildType == "sequential") {
		node(label : "${buildInfo.getBuilderLabel(projectName: projectName)}") {
			ws('workspace/pcg-fw') {
				checkout scm
				try {
					for (int i = 0; i < buildMatrix.size(); i++) {
						Map axis = buildMatrix[i]
						runSeqBuild(axis)
					}
				}
				catch (Exception ex) {
					echo "Failed in ${projectName}-Build"
					echo "Caught:${ex}"
					buildStatusInfo = "Failed"
					currentBuild.result = 'FAILURE'
				}
				deleteDir()
			}
		}
	}
	else {
		// Build all included matrix combinations
		buildMap = [:]
		for (int i = 0; i < buildMatrix.size(); i++) {
			// Convert the Axis into valid values for withEnv step
			Map axis = buildMatrix[i]
			List axisEnv = axis.collect { key, val ->
				"${key}=${val}"
			}

			buildMap[axisEnv.join(', ')] = { ->
				ws('workspace/pcg-fw') {
					checkout scm
					withEnv(axisEnv) {
						try {
							stage("Build-${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM}-${ACTIVE_DEMO_MODE_CONFIG}") {
								FIRMWARE_VERSION = gitCommand.gitCommitShort()

								echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
								echo "Running on node: '${env.NODE_NAME}'"
							
								echo "TOOLCHAIN: ${TOOLCHAIN}"
								echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
							
								echo "Building for ${PLATFORM_NAME} platform and ${ACTIVE_DEVICE} device with ${COM_TYPE} UART type, ${ACTIVE_DEMO_MODE_CONFIG} and ${SDRAM}"
								echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
							
								echo "Starting mbed build..."
								//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
								bat "cd ${projectName} & make test TARGET_BOARD=${PLATFORM_NAME} ARTIFACT-NAME=${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM}-${ACTIVE_DEMO_MODE_CONFIG} TEST_FLAGS=-DDEVICE_NAME=\\\\\\\"${ACTIVE_DEVICE}\\\\\\\" TEST_FLAGS+=-DPLATFORM_NAME=\\\\\\\"${PLATFORM_NAME}\\\\\\\" TEST_FLAGS+=-DFIRMWARE_VERSION=\\\\\\\"${FIRMWARE_VERSION}\\\\\\\" TEST_FLAGS+=-DACTIVE_DEMO_MODE_CONFIG=${ACTIVE_DEMO_MODE_CONFIG} TEST_FLAGS+=-D${ACTIVE_DEVICE} TEST_FLAGS+=-D${COM_TYPE} TEST_FLAGS+=-D${SDRAM}"
								artifactory.uploadArtifacts("${projectName}/build/${PLATFORM_NAME}/${TOOLCHAIN}","precision-converters-firmware/${projectName}/${env.BRANCH_NAME}")
								archiveArtifacts allowEmptyArchive: true, artifacts: "${projectName}/build/**/*.bin, ${projectName}/build/**/*.elf"
								stash includes: "${projectName}/build/**/*.bin, ${projectName}/build/**/*.elf", name: "${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM}-${ACTIVE_DEMO_MODE_CONFIG}"
								deleteDir()
							}
						}
						catch (Exception ex) {
								echo "Failed in Build-${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM}-${ACTIVE_DEMO_MODE_CONFIG} stage"
								echo "Caught:${ex}"
								buildStatusInfo = "Failed"
								currentBuild.result = 'FAILURE'
								deleteDir()
						}
					}
				}
			}
		}

		stage("Matrix Builds") {
			parallel(buildMap)
		}
	}

	return buildStatusInfo
}

def runSeqBuild(Map config =[:]) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}") {
		FIRMWARE_VERSION = gitCommand.gitCommitShort()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} platform and ${config.ACTIVE_DEVICE} device with ${config.COM_TYPE} UART type, ${config.ACTIVE_DEMO_MODE_CONFIG} and ${config.SDRAM}"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		bat "cd ${projectName} & make test TARGET_BOARD=${config.PLATFORM_NAME} ARTIFACT-NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG} TEST_FLAGS=-DDEVICE_NAME=\\\\\\\"${config.ACTIVE_DEVICE}\\\\\\\" TEST_FLAGS+=-DPLATFORM_NAME=\\\\\\\"${config.PLATFORM_NAME}\\\\\\\" TEST_FLAGS+=-DFIRMWARE_VERSION=\\\\\\\"${FIRMWARE_VERSION}\\\\\\\" TEST_FLAGS+=-DACTIVE_DEMO_MODE_CONFIG=${config.ACTIVE_DEMO_MODE_CONFIG} TEST_FLAGS+=-D${config.ACTIVE_DEVICE} TEST_FLAGS+=-D${config.COM_TYPE} TEST_FLAGS+=-D${config.SDRAM}"
		artifactory.uploadArtifacts("${projectName}/build/${config.PLATFORM_NAME}/${TOOLCHAIN}","precision-converters-firmware/${projectName}/${env.BRANCH_NAME}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "${projectName}/build/**/*.bin, ${projectName}/build/**/*.elf"
		stash includes: "${projectName}/build/**/*.bin, ${projectName}/build/**/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}"
		bat "cd ${projectName} & make clean TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def ad4130Test() {
	Map testMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for testing all targets when merging to main or develop branches
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['NO_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG']
		]
	} else {
		// This map is for testing target that is always tested
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD4130'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['NO_SDRAM'],
			ACTIVE_DEMO_MODE_CONFIG: ['USER_DEFAULT_CONFIG']
		]
	}

	List testMatrix = general.getMatrixCombination(testMap)
	for (int i = 0; i < testMatrix.size(); i++) {
		Map axis = testMatrix[i]
		runTest(axis)
	}
}

def runTest(Map config =[:]) {
	node(label : "${hw.getAgentLabel(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVB-REF-' + config.ACTIVE_DEVICE)}") {
		ws('workspace/pcg-fw') {
			checkout scm
			try {
				stage("Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}") {
					// Fetch the stashed files from build stage
					unstash "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}"

					echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"                       
					echo "Running on node: '${env.NODE_NAME}'"
					echo "Testing for ${config.PLATFORM_NAME} platform and ${config.ACTIVE_DEVICE} device with ${config.COM_TYPE} UART type, ${config.ACTIVE_DEMO_MODE_CONFIG} and ${config.SDRAM}"
					echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
					
					platformInfo = hw.getPlatformInfo(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVB-REF-' + config.ACTIVE_DEVICE)
					echo "Programming MCU platform..."               
					bat "mbedflsh --version"
					// need to ignore return code from mbedflsh as it returns 1 when programming successful
					bat returnStatus: true , script: "mbedflsh --disk ${platformInfo["mountpoint"]} --file ${WORKSPACE}\\${projectName}\\build\\${config.PLATFORM_NAME}\\${TOOLCHAIN}\\${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG}.bin"               
					bat "cd ${projectName}/tests & pytest --rootdir=${WORKSPACE}\\${projectName}\\tests -c pytest.ini --serialport ${platformInfo["serialport"]} --device_name ${config.ACTIVE_DEVICE} --platform_name ${config.PLATFORM_NAME} --serial_com_type ${config.COM_TYPE}"			
					archiveArtifacts allowEmptyArchive: true, artifacts: "${projectName}/tests/output/*.csv"
					junit allowEmptyResults:true, testResults: "**/${projectName}/tests/output/*.xml"
					deleteDir()
				}
			}
			catch (Exception ex) {
				echo "Failed in Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}-${config.ACTIVE_DEMO_MODE_CONFIG} stage"
				echo "Caught:${ex}"
				currentBuild.result = 'FAILURE'
				deleteDir()
			}
		}
	}
}

return this;