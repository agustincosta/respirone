cuentas_medidas =  [1165, 1320, 1452, 1603, 1774, 1935, 2100, 2284, 2436, 2583, 2653];
distancia_medida = [93-64.2, 93-60, 93.2-57, 93.2-52.8, 93-48.5, 93-44.4, 93-40.7, 93-36.3, 93-32.7, 93-29.5, 93-28.3];
distancia_teorica = [24.2, 28.5, 32.6, 36.7, 40.7, 44.8, 48.9, 53.0, 57.0, 61.1, 65.2];

c = cuentas_medidas.*distancia_teorica./distancia_medida;
c = round(c);

fprintf('%d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n %d,\n',c(1),c(2),c(3),c(4),c(5),c(6),c(7),c(8),c(9),c(10),c(11))