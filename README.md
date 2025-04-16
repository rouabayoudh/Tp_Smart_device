﻿# API documentation

base url = https://nodejs-serverless-patient-monitoring.vercel.app/api

## GET requests:

* get all patients: 

    base_url + /patients

* get one patient by id: 

    base_url + /patient?id=[id]


## POST requests:

* create a new patient: 


    base_url + /newPatient

  eg body:
      {
        "mac_address": "00:1A:2B:3C:4D:5E",
        "name": "Patient Name"
      }

  

* create a new sensor data (to an existing patient): 


    base_url + /newSensorData?mac_address=[mac_address]
  
  eg body:
      {
        "spo2": 95,
        "bpm": 70,
        "body_temp": 37.6,
        "timestamp": 2024-12-07 16:57:29.707387 (tajam ma t7othech w tkhaliha par défaut y7otha houa)
      }
  
  RQ: fl create a new sensor data lezmek t3adi l mac_address fl params for it to work

## PUT requests:

* change a patient's name:

  base_url + /updatePatientName

  eg body:
      {
      "mac_address": "00:1A:2B:3C:4D:5E",
      "name": "Updated Patient Name"
      }
 
#   T p _ S m a r t _ d e v i c e  
 