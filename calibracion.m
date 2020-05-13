cuentas_medidas =  [1162,1362,1460,1660,1802,1942,2089,2210,2353,2539,2697];
distancia_medida = 104.3 - [78.3,74.1,70.5,66.6,61.5,57.9,54.4,51.3,47.7,43.6,38.6];
distancia_teorica = [24.2, 28.5, 32.6, 36.7, 40.7, 44.8, 48.9, 53.0, 57.0, 61.1, 65.2];

c = cuentas_medidas.*distancia_teorica./distancia_medida;
c = round(c);

fprintf('%d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n',c(1),c(2),c(3),c(4),c(5),c(6),c(7),c(8),c(9),c(10),c(11))