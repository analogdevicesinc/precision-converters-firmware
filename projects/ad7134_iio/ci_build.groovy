def doBuild(projectName) {
	Map buildMap =[:]
	if (env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop") {
		// This map is for building all targets when merging to main or develop branches
		buildMap = [
			PLATFORM_NAME: ['SDP_K1', 'NUCLEO_L552ZE_Q', 'DISCO_F769NI'],
			ACTIVE_DEVICE: ['DEV_AD7134'],
			DATA_TRANSFER_PORT: ['USE_PHY_COM_PORT','USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DATA_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE', 'BURST_DATA_CAPTURE'],
			INTERFACE_MODE : ['BIT_BANGING_MODE']
		]
	} else {
		// This map is for building targets that is always build
		buildMap = [
			PLATFORM_NAME: ['SDP_K1'],
			ACTIVE_DEVICE: ['DEV_AD7134'],
			DATA_TRANSFER_PORT: ['USE_VIRTUAL_COM_PORT'],
			SDRAM: ['USE_SDRAM', 'NO_SDRAM'],
			DATA_CAPTURE_MODE: ['CONTINUOUS_DATA_CAPTURE'],
			INTERFACE_MODE : ['BIT_BANGING_MODE']
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
	}
	else {
		// Build all included matrix combinations
		buildMap = [:]
		for (int i = 0; i < buildMatrix.size(); i++) {
			// Convert the Axis into valid values for withEnv step
			Map axis = buildMatrix[i]
			List axisEnv = axis.collect { k, v ->
				"${k}=${v}"
			}

			buildMap[axisEnv.join(', ')] = { ->
				ws('workspace/pcg-fw') {
					checkout scm
					withEnv(axisEnv) {
						try {
							stage("Build-${PLATFORM_NAME}-${ACTIVE_DEVICE}-${DATA_TRANSFER_PORT}-${SDRAM}-${DATA_CAPTURE_MODE}-${INTERFACE_MODE}") {
								FIRMWARE_VERSION = buildInfo.getFirmwareVersion()

								echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"     
								echo "Running on node: '${env.NODE_NAME}'"
							
								echo "TOOLCHAIN: ${TOOLCHAIN}"
								echo "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}"
							
								echo "Building for ${PLATFORM_NAME} platform and ${ACTIVE_DEVICE} device with ${DATA_TRANSFER_PORT} UART type, ${DATA_CAPTURE_MODE} mode and ${INTERFACE_MODE} interface"
								echo "^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^ ^^**^^"
							
								echo "Starting mbed build..."
								//NOTE: if adding in --profile, need to change the path where the .bin is found by mbedflsh in Test stage
								bat "cd projects/${projectName} & make clone-lib-repos"
								bat "cd projects/${projectName} & make all LINK_SRCS=n TARGET_BOARD=${PLATFORM_NAME} BINARY_FILE_NAME=${PLATFORM_NAME}-${ACTIVE_DEVICE}-${DATA_TRANSFER_PORT}-${SDRAM}-${DATA_CAPTURE_MODE}-${INTERFACE_MODE} NEW_CFLAGS+=-DPLATFORM_NAME=${PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-DDATA_CAPTURE_MODE=${DATA_CAPTURE_MODE} NEW_CFLAGS+=-DINTERFACE_MODE=${INTERFACE_MODE} NEW_CFLAGS+=-D${DATA_TRANSFER_PORT} NEW_CFLAGS+=-D${SDRAM}"
								artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
								archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
								stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${PLATFORM_NAME}-${ACTIVE_DEVICE}-${DATA_TRANSFER_PORT}-${SDRAM}-${DATA_CAPTURE_MODE}-${INTERFACE_MODE}"
								buildStatusInfo = "Success"
								deleteDir()
							}
						}
						catch (Exception ex) {
								echo "Failed in Build-${PLATFORM_NAME}-${ACTIVE_DEVICE}-${DATA_TRANSFER_PORT}-${SDRAM}-${DATA_CAPTURE_MODE}-${INTERFACE_MODE} stage"
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
		bat "cd projects/${projectName} & make clone-lib-repos"
		bat "cd projects/${projectName} & make all LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME} BINARY_FILE_NAME=${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE} NEW_CFLAGS+=-DPLATFORM_NAME=${config.PLATFORM_NAME} NEW_CFLAGS+=-DFIRMWARE_VERSION=${FIRMWARE_VERSION} NEW_CFLAGS+=-DDATA_CAPTURE_MODE=${config.DATA_CAPTURE_MODE} NEW_CFLAGS+=-DINTERFACE_MODE=${config.INTERFACE_MODE} NEW_CFLAGS+=-D${config.DATA_TRANSFER_PORT} NEW_CFLAGS+=-D${config.SDRAM}"
		artifactory.uploadFirmwareArtifacts("projects/${projectName}/build","${projectName}")
		archiveArtifacts allowEmptyArchive: true, artifacts: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf"
		stash includes: "projects/${projectName}/build/*.bin, projects/${projectName}/build/*.elf", name: "${config.PLATFORM_NAME}-${config.ACTIVE_DEVICE}-${config.DATA_TRANSFER_PORT}-${config.SDRAM}-${config.DATA_CAPTURE_MODE}-${config.INTERFACE_MODE}"
		bat "cd projects/${projectName} & make reset LINK_SRCS=n TARGET_BOARD=${config.PLATFORM_NAME}"
	}
}

def doTest(projectName) {
	// TODO: CI test setup not done yet
}

return this;