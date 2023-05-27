use_module(library(http/json)).

json_params('{
    "type": "params",
    "wather_lewel": 100,    
    "probability_rain": 20,
    "temperature_celcius": 12,
    "humidity": 20
}').

weather_conditions(P, R, T, H) :-
    P < 30, H < 30.

check_weather :-
    json_params(JsonString),
    atom_json_dict(Json, JsonString, []),
    write(Json), nl,  % dodaj tę linię
    member(probability_rain=P, Json),
    member(humidity=H, Json),
    weather_conditions(P, _, _, H),
    write('True').
    
check_weather :- write('False').
