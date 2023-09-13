def doBuild(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD7606B', 'DEV_AD7606C_16', 'DEV_AD7606C_18', 'DEV_AD7608', 'DEV_AD7609', 'DEV_AD7606_8', 'DEV_AD7606_6', 'DEV_AD7606_4', 'DEV_AD7605_4'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7606B'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip 'all platforms except SDP-K1 + SDRAM and No SDRAM + VCOM'
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['SDRAM'] == 'USE_SDRAM') &&
    	!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['SDRAM'] == 'USE_SDRAM') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&

		// Skip 'all platforms except SDP-K1 + all devices except AD7606B'
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_16') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_18') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7608') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7609') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_8') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_6') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_4') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['ACTIVE_DEVICE'] == 'DEV_AD7605_4') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_16') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_18') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7608') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7609') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_8') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_6') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_4') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['ACTIVE_DEVICE'] == 'DEV_AD7605_4') &&

		// Skip 'SDP-K1 + SDRAM + Phy COM + all devices except DEV_AD7606B'
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_16' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_18' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7608' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7609' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_8' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_6' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_4' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7605_4' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&

		// Skip 'SDP-K1 + No SDRAM + Phy COM + all devices except DEV_AD7606B'
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_16' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_18' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7608' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7609' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_8' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_6' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_4' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7605_4' && axis['SDRAM'] == 'NO_SDRAM' && axis['COM_TYPE'] == 'USE_PHY_COM_PORT') &&

		// Skip 'SDP-K1 + SDRAM + VCOM + all devices except DEV_AD7606B'
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_16' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606C_18' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7608' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7609' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_8' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_6' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7606_4' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'SDP_K1' && axis['ACTIVE_DEVICE'] == 'DEV_AD7605_4' && axis['SDRAM'] == 'USE_SDRAM' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT')
	}

	node(label : "firmware_builder") {
		ws('workspace/pcg-fw') {
			checkout scm
			try {
				echo "Number of matrix combinations: ${buildMatrix.size()}"
				for (int i = 0; i < buildMatrix.size(); i++) {
					Map axis = buildMatrix[i]
					runSeqBuild(axis, projectName)
				}
				// This variable holds the build status
				buildStatusInfo = "Success"
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

	return buildStatusInfo
}

def runSeqBuild(Map config =[:], projectName) {
	stage("Build-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}") {
		FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
		echo "Running on node: '${env.NODE_NAME}'"
	
		echo "TOOLCHAIN: ${TOOLCHAIN}"
		echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
	
		echo "Building for ${config.PLATFORM_NAME} Platform and ${config.ACTIVE_DEVICE} Device with ${config.COM_TYPE} UART type and ${config.SDRAM}"
		echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
	
		echo "Starting mbed build..."
		//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
		sh "cd projects/${projectName} ; make clone-lib-repos"
		sh "cd projects/${projectName} ; make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM} NEW_CFLAGS+=-DPLATFORM_NAME=${config.PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-D${config.ACTIVE_DEVICE} NEW_CFLAGS+=-D${config.COM_TYPE} NEW_CFLAGS+=-D${config.SDRAM}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}"
		sh "cd projects/${projectName} ; make reset LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def doTest(projectName) {
	Map testMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for testing all targets when merging to main or develop branches
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7606B'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['NO_SDRAM']
		]
	} else {
		// This map is for testing target that is always tested
		testMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7606B'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['NO_SDRAM']
		]
	}

	List testMatrix = buildInfo.getMatrixCombination(testMap)
	for (int i = 0; i < testMatrix.size(); i++) {
		Map axis = testMatrix[i]
		runTest(axis, projectName)
	}
}

def runTest(Map config =[:], projectName) {
	node(label : "${hw.getAgentLabel(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVB-REF-' + config.ACTIVE_DEVICE)}") {
		ws('workspace/pcg-fw') {
			checkout scm
			try {
				stage("Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}") {
					// Fetch the stashed files from build stage
					unstash "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}"

					echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"                       
					echo "Running on node: '${env.NODE_NAME}'"
					echo "Testing for ${config.PLATFORM_NAME} and ${config.ACTIVE_DEVICE} device with ${config.COM_TYPE} UART type and ${config.SDRAM}"
					echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
					
					platformInfo = hw.getPlatformInfo(platform_name: config.PLATFORM_NAME, evb_ref_id: 'EVB-REF-' + config.ACTIVE_DEVICE)
					echo "Programming MCU platform..."               
					bat "mbedflsh --version"
					// need to ignore return code from mbedflsh as it returns 1 when programming successful
					bat returnStatus: true , script: "mbedflsh --disk ${platformInfo["mountpoint"]} --file ${WORKSPACE}\\projects\\${projectName}\\build\\${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}.bin"               
					bat "cd projects/${projectName}/tests & pytest --rootdir=${WORKSPACE}\\projects\\${projectName}\\tests -c pytest.ini --serialport ${platformInfo["serialport"]} --device_name ${config.ACTIVE_DEVICE} --platform_name ${config.PLATFORM_NAME} --serial_com_type ${config.COM_TYPE}"			
					archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/tests/output/*.csv"
					junit allowEmptyResults:true, testResults: "projects/${projectName}/tests/output/*.xml"
					deleteDir()
				}
			}
			catch (Exception ex) {
				echo "Failed in Test-${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM} stage"
				echo "Caught:${ex}"
				currentBuild.result = 'FAILURE'
				deleteDir()
			}
		}
	}
}

return this;