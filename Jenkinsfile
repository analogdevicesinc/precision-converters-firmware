@Library(['jenkins_shared_lib','pcts_jenkins_shared_lib']) _

groovyScriptsList = []
projectsNameList = []
buildStatus = "Success"

pipeline {
    agent none
	environment {
        // Compiler type used is currently hard coded
        TOOLCHAIN = "GCC_ARM"
        // MBED_GCC_TOOLCHAIN_PATH is a global jenkins defined environment variable
        TOOLCHAIN_PATH = "${MBED_GCC_TOOLCHAIN_PATH}"
	}
    options {
        // This stops multiple copies of this job running, but not multiple parallel matrix builds
        disableConcurrentBuilds() 

        // keeps some named stashes around so that pipeline stages can be restarted
        preserveStashes(buildCount: 5)

        // Set build log and artifact discarding policy
        buildDiscarder(
            logRotator(
                // number of build logs to keep
                numToKeepStr:'10',
                // number of builds have their artifacts kept
                artifactNumToKeepStr: '10'
            )
         )     
    }

    stages {
        stage("Load Build") {   // Prepare/load build scripts
			agent { label 'firmware_builder' }
			steps {
				script {
                    // Find the project changes
                    projectChanges = buildInfo.findProjectChanged()

                    // Get list of all the projects
                    def projectsList = []
                    def output = buildInfo.getProjectsList("${env.WORKSPACE}\\projects")
                    projectsList = output.tokenize('\n').collect() { it }

                    // Load project specific groovy scripts based on the project change
                    int cnt = 0
                    for (String projectName: projectsList) {
                        projectName = projectName.trim()
                        if (projectChanges.contains("$projectName") || projectChanges.contains("Jenkinsfile") || projectChanges.contains("libraries") || env.BRANCH_NAME=="main" || env.BRANCH_NAME=="develop") {
                            groovyScriptsList[cnt] = load "projects\\$projectName\\ci_build.groovy"
                            projectsNameList[cnt] = projectName
                            cnt++
                        }
                    }
                }
			}
			post {
				cleanup {
                    cleanWs()
                }
			}
        }

        stage("Build and Test") {   // Run build and test scripts
            agent { label 'firmware_builder' }
			steps {
				script {
                    // Checkout all submodules recursively
                    sh "git submodule update --init --recursive"

                    // Execute build and test job for each changed project
                    for (int cnt=0; cnt < groovyScriptsList.size(); cnt++) {
                        try {
                            // Get all build matrix combinations for current project
                            List buildMatrix = groovyScriptsList[cnt].getBuildMatrix("${projectsNameList[cnt]}");

                            // Build all matrix combinations for current project
                            echo "Number of build matrix combinations: ${buildMatrix.size()}"
                            for (int i = 0; i < buildMatrix.size(); i++) {
                                Map axis = buildMatrix[i]
                                groovyScriptsList[cnt].doBuild(axis, "${projectsNameList[cnt]}")
                            } 
                        }
                        catch (Exception ex) {
                            echo "Failed in ${projectsNameList[cnt]}-Build"
                            echo "Caught:${ex}"
                            currentBuild.result = 'FAILURE'
                            buildStatus = 'Failure'
                        }

                        if (buildStatus != 'Failure') {
                            try {
                                // Get all test matrix combinations for current project
                                List testMatrix = groovyScriptsList[cnt].getTestMatrix("${projectsNameList[cnt]}");

                                // Test all matrix combinations for current project
                                echo "Number of test matrix combinations: ${testMatrix.size()}"
                                for (int i = 0; i < testMatrix.size(); i++) {
                                    Map axis = testMatrix[i]
                                    groovyScriptsList[cnt].doTest(axis, "${projectsNameList[cnt]}")
                                }
                            }
                            catch (Exception ex) {
                                echo "Failed in ${projectsNameList[cnt]}-Test"
                                echo "Caught:${ex}"
                                currentBuild.result = 'FAILURE'
                            }
                        }
                    }
                }
			}
            post {
				cleanup {
                    cleanWs()
                }
			}
        }

		stage('Check Licensing') {  // Perform licensing scan
            when {
               expression { env.BRANCH_NAME == "main" || env.BRANCH_NAME == "develop" }
            }
            agent { label 'docker' }
            steps {
                script {
                    licensing.check()
                }
            }
        }	
    }
	
    post {
        always {
            // nothing to do
            echo "always"
        }
        success {
            // nothing to do
            echo "success"
        }
         failure {
            echo "failure"
        }
       unstable {
            // nothing to do
            echo "unstable"
        }
        changed {
            // nothing to do
            echo "changed"
        }
    }
}
