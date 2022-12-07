// This variable holds the build status
buildStatusInfo = "Success"

def doBuild(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD7380_2'],
			COM_TYPE: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7380_2'],
			COM_TYPE: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM']
		]
	}

	// Get the matrix combination and filter them to exclude unrequired matrixes
	List buildMatrix = buildInfo.getMatrixCombination(buildMap).findAll { axis ->
		// Skip 'all platforms except SDP-K1 + SDRAM and + VCOM'
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['SDRAM'] == 'USE_SDRAM') &&
    	!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['SDRAM'] == 'USE_SDRAM') &&
		!(axis['PLATFORM_NAME'] == 'NUCLEO_L552ZE_Q' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT') &&
		!(axis['PLATFORM_NAME'] == 'DISCO_F769NI' && axis['COM_TYPE'] == 'USE_VIRTUAL_COM_PORT')
	}

	def buildType = buildInfo.getBuildType()
	if (buildType == "sequential") {
		node(label : "${buildInfo.getBuilderLabel(projectName: projectName)}") {
			ws('workspace/pcg-fw') {
				checkout scm
				try {
					for (int i = 0; i < buildMatrix.size(); i++) {
						Map axis = buildMatrix[i]
						runSeqBuild(axis, projectName)
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
							stage("Build-${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM}") {
								FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

								echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
								echo "Running on node: '${env.NODE_NAME}'"
							
								echo "TOOLCHAIN: ${TOOLCHAIN}"
								echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
							
								echo "Building for ${PLATFORM_NAME} Platform and ${ACTIVE_DEVICE} Device with ${COM_TYPE} UART type and ${SDRAM}"
								echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
							
								echo "Starting mbed build..."
								//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
								bat "cd projects/${projectName} & make test TARGET_BOARD=${PLATFORM_NAME} ARTIFACT-NAME=${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM} TEST_FLAGS=-DDEVICE_NAME=\\\\\\\"${ACTIVE_DEVICE}\\\\\\\" TEST_FLAGS+=-DPLATFORM_NAME=\\\\\\\"${PLATFORM_NAME}\\\\\\\" TEST_FLAGS+=-DFIRMWARE_VERSION=\\\\\\\"${FIRMWARE_VERSION}\\\\\\\" TEST_FLAGS+=-D${ACTIVE_DEVICE} TEST_FLAGS+=-D${COM_TYPE} TEST_FLAGS+=-D${SDRAM}"
								artifactory.uploadFirmwareArtifacts("projects/${projectName}/build/${PLATFORM_NAME}/${TOOLCHAIN}","${projectName}")
								archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/**/*.bin, projects/${projectName}/build/**/*.elf"
								stash includes: "projects/${projectName}/build/**/*.bin, projects/${projectName}/build/**/*.elf", name: "${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM}"
								deleteDir()
							}
						}
						catch (Exception ex) {
								echo "Failed in Build-${PLATFORM_NAME}-${ACTIVE_DEVICE}-${COM_TYPE}-${SDRAM} stage"
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
		bat "cd projects/${projectName} & make test TARGET_BOARD=${config.PLATFORM_NAME} ARTIFACT-NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM} TEST_FLAGS=-DDEVICE_NAME=\\\\\\\"${config.ACTIVE_DEVICE}\\\\\\\" TEST_FLAGS+=-DPLATFORM_NAME=\\\\\\\"${config.PLATFORM_NAME}\\\\\\\" TEST_FLAGS+=-DFIRMWARE_VERSION=\\\\\\\"${FIRMWARE_VERSION}\\\\\\\" TEST_FLAGS+=-D${config.ACTIVE_DEVICE} TEST_FLAGS+=-D${config.COM_TYPE} TEST_FLAGS+=-D${config.SDRAM}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build/${config.PLATFORM_NAME}/${TOOLCHAIN}","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/**/*.bin, projects/${projectName}/build/**/*.elf"
		stash includes: "projects/${projectName}/build/**/*.bin, projects/${projectName}/build/**/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.COM_TYPE}-${config.SDRAM}"
		bat "cd projects/${projectName} & make clean TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def doTest(projectName) {
	// TODO Test setup not connected yet
}

return this;