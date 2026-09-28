pipeline {
    agent any

    stages {
        stage('Checkout Code') {
            steps{
                git branch: 'main', url: 'https://github.com/Ahmed-Mazen/WeatherStation.git'
            }
        }
        stage('Build'){
            steps{
                script {
                    docker.build("ahmedbahaamazen/weather-station:$BUILD_NUMBER")
                }
            }
        }
        stage('Test') {
            parallel {
                stage('Unit Test'){
                    steps{
                        sh ('docker run -d ahmedbahaamazen/weather-station:$BUILD_NUMBER')
                    }
                }
                stage('Integration test'){
                    steps{
                        echo 'Integration test'
                    }
                }
                stage('E2E testing'){
                    stages{
                        stage('E2E testing: backend'){
                            steps{
                                echo 'test'
                            }
                        }
                        stage('E2E testing: frontend'){
                            steps{
                                echo 'test'
                            }
                        }
                        stage('E2E testing: database'){
                            steps{
                                echo 'test'
                            }
                        }
                    }
                }
            }
        }
        stage('Release') {
            steps {
                withCredentials([usernamePassword(credentialsId: 'docker', passwordVariable: 'DockerPass', usernameVariable: 'DockerUser')]) {
                    sh "echo '$DockerPass' | docker login -u $DockerUser --password-stdin"
                    sh 'docker push ahmedbahaamazen/weather-station:$BUILD_NUMBER'
                }
            }
        }
        stage('deploy') {
            steps {
                sh 'docker step app 2> /dev/null || true'
                sh 'docker rm -f app 2> /dev/null || true'
                sh 'docker run --name app -d -p 80:80 ahmedbahaamazen/weather-station:$BUILD_NUMBER'
            }
        }
    }
}
