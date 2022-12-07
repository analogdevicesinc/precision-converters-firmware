@Library(['jenkins_shared_lib','pcts_jenkins_shared_lib']) _

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
        stage("Load Build") {
			agent { 
				node {
					label 'firmware_builder'  
				}
			}
			steps {
				script {
					general = load 'CI\\scripts\\generic_ci.groovy'
					general.findProjectChanged()
				}
			}
			post {
				cleanup {
                    cleanWs()
                }
			}
        }

		stage('Check Licensing') {
            when {  // When merging to main or develop branches, run licensing check
               expression { env.CHANGE_TARGET == "main" || env.CHANGE_TARGET == "develop"  }
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
