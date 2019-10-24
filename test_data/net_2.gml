graph [
  directed 1
  tick 300.0
  node [
    id 1
    age 53.08219178
    age_started 16.25019661
    race "Hispanic"
    gender "MALE"
    syringe_source "nonHR"
    zipcode "60647"
    drug_in_deg 6
    drug_out_deg 2
    inject_intens 0.84581825
    frac_recept 0.55828632
    lat 41.87562463
    lon -87.66221528
    last_exposure_date 0.00000000
    last_infection_date -1.00000000
    active 1
    deactivate_at 13462.50722435
    immunology [
    hcv_state "cured"
    past_cured 0
    past_recovered 0
    in_treatment 0
    treatment_start_date 713.30000000
    treatment_failed 0
    events [
    event_0 [
    canceled 0
    scheduled_for 796.64549429
    ef_type 2
    success 1
     ]
     ]
    params [
    mean_days_acute_naive 102.00000000
    mean_days_acute_rechallenged 28.00000000
    mean_days_naive_to_infectious 3.00000000
    mean_days_residual_hcv_infectivity 5.00000000
    prob_self_limiting_female 0.34600000
    prob_self_limiting_male 0.12100000
    prob_clearing 0.85000000
    transmissibility 0.01000000
    treatment_duration 84.00000000
    treatment_svr 0.90000000
    treatment_susceptibility 1.00000000
    treatment_repeatable 0
     ]
     ]
  ]
  node [
    id 2
    age 68.41643836
    age_started 17.03769982
    race "NHBlack"
    gender "MALE"
    syringe_source "nonHR"
    zipcode "60640"
    drug_in_deg 0
    drug_out_deg 1
    inject_intens 2.00002044
    frac_recept 0.26795001
    lat 41.92933201
    lon -87.66160738
    last_exposure_date 0.00000000
    last_infection_date -1.00000000
    active 1
    deactivate_at 320.50722435
    immunology [
    hcv_state "chronic"
    past_cured 0
    past_recovered 0
    in_treatment 0
    treatment_start_date -1.00000000
    treatment_failed 0

    params [
    mean_days_acute_naive 102.00000000
    mean_days_acute_rechallenged 28.00000000
    mean_days_naive_to_infectious 3.00000000
    mean_days_residual_hcv_infectivity 5.00000000
    prob_self_limiting_female 0.34600000
    prob_self_limiting_male 0.12100000
    prob_clearing 0.85000000
    transmissibility 0.01000000
    treatment_duration 84.00000000
    treatment_svr 0.90000000
    treatment_susceptibility 1.00000000
    treatment_repeatable 0
     ]
     ]
  ]

 node [
    id 3
    age 51.24657535
    age_started 30.39743820
    race "Hispanic"
    gender "MALE"
    syringe_source "nonHR"
    zipcode "60647"
    drug_in_deg 30
    drug_out_deg 0
    inject_intens 2.88931634
    frac_recept 0.41437703
    lat 41.92366212
    lon -87.72396575
    last_exposure_date 278.00000000
    last_infection_date 278.00000000
    active 1
    deactivate_at 2430.50722435
    immunology [
    hcv_state "recovered"
    past_cured 0
    past_recovered 1
    in_treatment 0
    treatment_start_date -1.00000000
    treatment_failed 0
    events [
    event_0 [
    canceled 0
    scheduled_for 302.72175123
    ef_type 0
     ]
    event_1 [
    canceled 0
    scheduled_for 310.20478304
    ef_type 1
     ]
     ]
    
    params [
    mean_days_acute_naive 102.00000000
    mean_days_acute_rechallenged 28.00000000
    mean_days_naive_to_infectious 3.00000000
    mean_days_residual_hcv_infectivity 5.00000000
    prob_self_limiting_female 0.34600000
    prob_self_limiting_male 0.12100000
    prob_clearing 0.85000000
    transmissibility 0.01000000
    treatment_duration 84.00000000
    treatment_svr 0.90000000
    treatment_susceptibility 1.00000000
    treatment_repeatable 0
     ]
     ]
  ]

node [
    id 4
    age 42.73972603
    age_started 29.00000000
    race "NHWhite"
    gender "MALE"
    syringe_source "HR"
    zipcode "60657"
    drug_in_deg 0
    drug_out_deg 0
    inject_intens 1.66666667
    frac_recept 0.00000000
    lat 41.95766352
    lon -87.64600086
    last_exposure_date -1.00000000
    last_infection_date -1.00000000
    active 1
    deactivate_at 2300.0
    immunology [
    hcv_state "cured"
    past_cured 0
    past_recovered 0
    in_treatment 1
    treatment_start_date 386.30000000
    treatment_failed 0
    events [
    event_0 [
    canceled 0
    scheduled_for 469.35608272
    ef_type 2
    success 1
     ]
     ]
    params [
    mean_days_acute_naive 102.00000000
    mean_days_acute_rechallenged 28.00000000
    mean_days_naive_to_infectious 3.00000000
    mean_days_residual_hcv_infectivity 5.00000000
    prob_self_limiting_female 0.34600000
    prob_self_limiting_male 0.12100000
    prob_clearing 0.85000000
    transmissibility 0.01000000
    treatment_duration 84.00000000
    treatment_svr 0.90000000
    treatment_susceptibility 1.00000000
    treatment_repeatable 0
     ]
     ]
  ]

  edge [
    source 3
    target 1
    distance 12.34300000
    ends_at 480.1 
  ]
  edge [
    source 1
    target 2
    distance 2.34300000
    ends_at 302.1
  ]
  edge [
    source 1
    target 4
    distance 1832.00000000
    ends_at 2000.1
  ]
  edge [
    source 3
    target 4
    distance 324.00000000
    ends_at 324.1
  ]

  edge [
    source 4
    target 3
    distance 324.00000000
    ends_at 1002.1
  ]
]
