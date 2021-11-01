unit Main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, Grids, StdCtrls, Buttons, ExtCtrls, Placemnt, Mask, ToolEdit,
  Db, RxMemDS, DBGrids, RXDBCtrl;

type
  TfrmMain = class(TForm)
    Panel1: TPanel;
    btStart: TSpeedButton;
    btClose: TBitBtn;
    edBitLen: TEdit;
    Label1: TLabel;
    Panel2: TPanel;
    StatusBar1: TStatusBar;
    btStop: TSpeedButton;
    edMinCorr: TEdit;
    Label2: TLabel;
    FilenameEdit1: TFilenameEdit;
    Label3: TLabel;
    FormStorage1: TFormStorage;
    btSave: TSpeedButton;
    RxDBGrid1: TRxDBGrid;
    MemData: TRxMemoryData;
    DataSource1: TDataSource;
    btGraph: TSpeedButton;
    fldCod: TIntegerField;
    fldCorrMin: TIntegerField;
    fldCorrMax: TIntegerField;
    fldCrossCorManMin: TIntegerField;
    fldCrossCorManMax: TIntegerField;
    procedure btStartClick(Sender: TObject);
    procedure btStopClick(Sender: TObject);
    procedure btSaveClick(Sender: TObject);
    procedure RxDBGrid1TitleBtnClick(Sender: TObject; ACol: Integer;
      Field: TField);
    procedure btGraphClick(Sender: TObject);
  private
    fstop:boolean;
    resarr:array of array[0..4] of integer;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmMain: TfrmMain;

implementation

uses Graph;

{$R *.DFM}

procedure TfrmMain.btStartClick(Sender: TObject);
var
    i,j:longint;
    mask,mask2:longint;
    bitlen:integer;
    evpopularr:array[0..255] of byte;
    k,l:integer;
    bitchain2, bitchain1:longint;
    res:longint;
    corr:integer;
    maxcorr,mincorr,maxcorr2,mincorr2:integer;
    limcorr:integer;
    rowcnt:integer;
    s:string;

begin
  screen.Cursor:=crHourGlass;
  try
    fstop:=false;
    Memdata.EmptyTable;
    bitlen:=StrToInt(edBitLen.text);
    limcorr:=StrToInt(edMinCorr.text);
    rowcnt:=1;
    mask:=0;
    SetLength(resarr,1);

    for i:=1 to bitlen do
      mask:=(mask shl 1) or 1;
    // Сформировать массив вычисления популяции
    for i:=0 to 255 do
    begin
      k:=0;
      for j:=0 to 7 do
        k:=k+((i shr j) and 1);
      evpopularr[i]:=k;
    end;
    // Начать перебор
    StatusBar1.Panels[0].Text:='Перебор вариантов...';
    Application.ProcessMessages;
    for i:=0 to mask do
    begin
//      if i=1335 then
//        fstop:=true;

      // Расчитать автокорреляцию для всех сдвигов
      bitchain1:=i;
      bitchain2:=i;
      maxcorr:=0;
      mincorr:=bitlen;
      l:=bitlen;
      mask2:=mask;
      for j:=1 to bitlen-1 do
      begin
        bitchain2:=bitchain2 shr 1;
        mask2:=mask2 shr 1;
        l:=l-1;
        res:=(bitchain2 xor bitchain1) and mask2;
        corr:=l-2*(evpopularr[res and 255]+ evpopularr[(res shr 8) and 255]+
              evpopularr[(res shr 16) and 255]+ evpopularr[(res shr 24) and 255]);
        if corr>maxcorr then maxcorr:=corr;
        if corr<mincorr then mincorr:=corr;

      end;
      if maxcorr<limcorr then
      begin
        SetLength(resarr,rowcnt);
        resarr[rowcnt-1,0]:=bitchain1;
        resarr[rowcnt-1,1]:=mincorr;
        resarr[rowcnt-1,2]:=maxcorr;
        rowcnt:=rowcnt+1;
      end;
      Application.ProcessMessages;
      if fstop then abort;

    end;
    // Для выбранного подмножества определить максимальную кросскорреляцию с манчестерским кодом
    // с вдвое меньшей скоростью передачи
    StatusBar1.Panels[0].Text:='Корреляция с манч.кодом...';
    Application.ProcessMessages;

    // Начать перебор массива результатов
    for i:=0 to  Length(resarr)-1 do
    begin
      maxcorr2:=0;
      mincorr2:=bitlen;
      // перебрать все возможные коды
      for j:=0 to 255 do
      begin
        // сформировать код
        bitchain1:=0;
        for k:=0 to 7 do
          if ((j shr k) and 1)=1 then bitchain1:=(bitchain1 shl 2) or 2 else  bitchain1:=(bitchain1 shl 2) or 1;
        // Расчитать кросскорреляцию с кодом
        maxcorr:=0;
        mincorr:=bitlen;
        l:=bitlen;
        bitchain2:=resarr[i,0];
        mask2:=mask;
        // Сдвигаем манчестерский код вперед
        for k:=0 to bitlen-1 do
        begin
          res:=(bitchain2 xor bitchain1) and mask2;
          corr:=l-2*(evpopularr[res and 255]+ evpopularr[(res shr 8) and 255]+
                evpopularr[(res shr 16) and 255]+ evpopularr[(res shr 24) and 255]);
          // Сдвинуть исходную последовательность
          bitchain2:=bitchain2 shr 1;
          mask2:=mask2 shr 1;
          l:=l-1;
          if corr>maxcorr then  maxcorr:=corr;
          if corr<mincorr then  mincorr:=corr;
          Application.ProcessMessages;
          if fstop then abort;
        end;

        l:=bitlen;
        bitchain2:=resarr[i,0];
        mask2:=mask;
        // Сдвигаем манчестерский код назад
        for k:=1 to bitlen-1 do
        begin
          // Сдвинуть исходную последовательность
          bitchain1:=bitchain1 shr 1;
          mask2:=mask2 shr 1;
          l:=l-1;

          res:=(bitchain2 xor bitchain1) and mask2;
          corr:=l-2*(evpopularr[res and 255]+ evpopularr[(res shr 8) and 255]+
                evpopularr[(res shr 16) and 255]+ evpopularr[(res shr 24) and 255]);

          if corr>maxcorr then  maxcorr:=corr;
          if corr<mincorr then  mincorr:=corr;
          Application.ProcessMessages;
          if fstop then abort;
        end;


        if maxcorr>maxcorr2 then maxcorr2:=maxcorr;
        if mincorr<mincorr2 then mincorr2:=mincorr;

      end;
      //Закончен перебор кодов
      resarr[i,4]:=maxcorr2;
      resarr[i,3]:=mincorr2;

    end;
     // Вывести на экран
    StatusBar1.Panels[0].Text:='Вывод в таблицу...';
    Application.ProcessMessages;

    for i:=0 to  Length(resarr)-1 do
    begin
      bitchain1:=resarr[i,0];
      s:='';
      for j:=0 to bitlen-1 do
      begin
        if  ((bitchain1 shr j) and 1)=1 then s:='1'+s else s:='0'+s;
      end;
      Memdata.Append;
      Memdata.Fields[0].value:= resarr[i,0];
      Memdata.Fields[1].value:= resarr[i,1];
      Memdata.Fields[2].value:= resarr[i,2];
      Memdata.Fields[3].value:= resarr[i,3];
      Memdata.Fields[4].value:= resarr[i,4];
      Memdata.UpdateRecord;

    end;

  finally
    StatusBar1.Panels[0].Text:='';
    screen.Cursor:=crdefault;
  end;

end;


procedure TfrmMain.btStopClick(Sender: TObject);
begin
  fstop:=true;
end;

procedure TfrmMain.btSaveClick(Sender: TObject);
var FileStream:TFileStream;
    i,j,bitchain1:longint;
    s:string;
    bitlen:integer;
    l:integer;
    ps:Pchar;
begin
  FileStream:=TFileStream.Create(FileNameEdit1.Text,fmCreate);
  try
    bitlen:=StrToInt(edBitLen.text);
    for i:=0 to  Length(resarr)-1 do
    begin
      bitchain1:=resarr[i,0];
      s:='';
      for j:=0 to bitlen-1 do
      begin
        if  ((bitchain1 shr j) and 1)=1 then s:='1'+s else s:='0'+s;
      end;
      s:=s+' '+Format('%d',[resarr[i,1]])+' '+Format('%d',[resarr[i,2]])+' '+
               Format('%d',[resarr[i,3]])+' '+Format('%d',[resarr[i,4]]);
      s:=s+Chr($0a);
      l:= length(s);
      ps:=PChar(s);
      FileStream.WriteBuffer(ps^,l);
    end;

  finally
    FileStream.Free;
  end;
end;

procedure TfrmMain.RxDBGrid1TitleBtnClick(Sender: TObject; ACol: Integer;
  Field: TField);
begin
  MemData.SortOnFields(Field.fullname,true,true);
end;

procedure TfrmMain.btGraphClick(Sender: TObject);
var
  A:array of double;
  i,j,k,l,corr:longint;
  mask,mask2,bitchain1,bitchain2,res:longint;
  bitlen:integer;
  evpopularr:array[0..255] of byte;
begin
  try
    for i:=0 to 255 do
    begin
      k:=0;
      for j:=0 to 7 do k:=k+((i shr j) and 1);
      evpopularr[i]:=k;
    end;

    bitlen:=StrToInt(edBitLen.text);
    SetLength(A,bitlen);
    mask:=0;
    for i:=1 to bitlen do mask:=(mask shl 1) or 1;
    bitchain1:=Memdata.Fields[0].value;
    bitchain2:=bitchain1;
    mask2:=mask;
    l:=bitlen;
    for j:=0 to bitlen-1 do
    begin
      res:=(bitchain2 xor bitchain1) and mask2;
      corr:=l-2*(evpopularr[res and 255]+ evpopularr[(res shr 8) and 255]+
            evpopularr[(res shr 16) and 255]+ evpopularr[(res shr 24) and 255]);

      A[j]:=corr;
      bitchain2:=bitchain2 shr 1;
      mask2:=mask2 shr 1;
      l:=l-1;
    end;
    if frmGraph=nil then frmGraph:=TfrmGraph.Create(Self);

    frmGraph.Show;
    with frmGraph do
    begin
      Series1.Clear;
      For i:=0 to Length(A)-1 do
      begin
        Series1.AddXY(i,A[i]);
      end;
      Series1.Repaint; 
    end;
  except

  end;


end;

end.
