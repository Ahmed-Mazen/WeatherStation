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
            steps{
                sh ('docker run -d -p 80:80 ahmedbahaamazen/weather-station:$BUILD_NUMBER')
            }
        }
        stage('Deploy') {
            steps {
                withCredentials([usernamePassword(credentialsId: 'docker', passwordVariable: 'DockerPass', usernameVariable: 'DockerUser')]) {
                    sh "echo '$DockerPass' | docker login -u $DockerUser --password-stdin"
                    sh 'docker push ahmedbahaamazen/weather-station:$BUILD_NUMBER'
                }
            }
        }
    }
}
