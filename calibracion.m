cuentas_medidas =  [1285, 1433, 1557, 1692, 1848, 1998, 2158, 2270,2444,2587,2731];
distancia_medida = 100.3 - [69.9,66.1,63.3,59.6,55.7,51.9,48.0,45.0,40.5,37.2,33.6];
distancia_teorica = [24.2, 28.5, 32.6, 36.7, 40.7, 44.8, 48.9, 53.0, 57.0, 61.1, 65.2];

c = cuentas_medidas.*distancia_teorica./distancia_medida;
c = round(c);

fprintf('%d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n',c(1),c(2),c(3),c(4),c(5),c(6),c(7),c(8),c(9),c(10),c(11))