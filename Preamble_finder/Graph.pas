unit Graph;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Placemnt, TeeComma, ExtCtrls, TeEngine, Series, TeeProcs, Chart;

type
  TfrmGraph = class(TForm)
    Chart1: TChart;
    Series1: TPointSeries;
    Panel1: TPanel;
    TeeCommander1: TTeeCommander;
    FormStorage1: TFormStorage;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmGraph: TfrmGraph;

implementation

{$R *.DFM}

end.
