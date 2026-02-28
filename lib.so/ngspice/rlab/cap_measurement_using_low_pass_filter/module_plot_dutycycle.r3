//
//
//
gnuwin(1);
gnulegend(["R1/k{/Symbol W}=",""]+members(range_vcap));
gnuxtics (1/8,1);
gnuytics (0.5,5);
gnuplot(range_vcap);

gnuwin(2);
gnulegend("R1/k{/Symbol W}=5");
gnuplot(range_vcap.["05.00"]);
