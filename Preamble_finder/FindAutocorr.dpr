program FindAutocorr;

uses
  Forms,
  Main in 'Main.pas' {frmMain},
  Graph in 'Graph.pas' {frmGraph};

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TfrmMain, frmMain);
  Application.Run;
end.
