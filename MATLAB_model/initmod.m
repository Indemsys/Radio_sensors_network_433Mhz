function []=prepare_rf_model()
%  Initializer RF model

    Ftr = 2000;     
    k=8; % Oversample            
    %Frc=21000
    Frc = Ftr*k;    
    pack_len=11; 
    pack_num=1000;
    
    sig_noice = 0.1;
    pause_noice = 0.5;
    
    Preamble=[1,1,0,1,1,0,0,1,1,1,0,0,0,0,1,0]; %[1,1,1,1,1,0,0,1,1,0,1,0,1];%[1,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1];% 
    %Preamble=[1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0];
    %Preamble=[1,1,0,0,0,1,0,1,1,1,0,0,0,0,1,0];
    % Barker 13 (1,1,1,1,1,0,0,1,1,0,1,0,1);
    Manchester_crosscorrelation=8;
    Preamble_pat=Create_pat(Preamble,1/(Ftr*2),1/Frc);
    Preamble_pat_len=length(Preamble_pat);
    Preamble_decis_levl=((Preamble_pat_len-Manchester_crosscorrelation*k/2)/2+Preamble_pat_len/2+Preamble_pat_len)/2-3;

    Bit_pat=Create_pat([1,0],1/(Ftr*2)-1/(2*Frc),1/Frc);
    assignin('base','Ftr',Ftr);
    assignin('base','k',k);
    assignin('base','Frc',Frc);
    assignin('base','Preamble',Preamble);
    assignin('base','Preamble_pat',Preamble_pat);
    assignin('base','Preamble_pat_len',Preamble_pat_len);
    assignin('base','Bit_pat',Bit_pat);
    assignin('base','Bit_pat_len',length(Bit_pat));
    assignin('base','Preamble_decis_levl',Preamble_decis_levl);
    assignin('base','pack_len',pack_len);
    assignin('base','pack_num',pack_num);

    assignin('base','sig_noice',sig_noice);
    assignin('base','pause_noice',pause_noice);

 %   
    function [templ_patrn]=Create_pat(templ,t_tmpl,t_pat)
      L=length(templ);
      t=L*t_tmpl;       
      n=fix(t/t_pat); % evalute number of point in pattern
      k=1;
      for i=0:(n-1)
        while (i*t_pat)>=(k*t_tmpl)
          if (k==L) 
              break; 
          end
          k=k+1;      
        end    
        templ_patrn((i+1),1)=templ(k);    
      end    
     