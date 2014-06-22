var sock = new SockJS('http://'+location.host+'/robot');
function RobotCtrl($scope) {
  $scope.messages = [];
  $scope.sendCommand = function(cmd) {
    sock.send(cmd || $scope.commandText);
    $scope.commandText = "";
  };

  sock.onmessage = function(e) {
    $scope.messages.push(e.data);
    $scope.$apply();
  };
  
  key('e', function(){ $scope.sendCommand('a'); });
  key('q', function(){ $scope.sendCommand('q'); });
  key('w', function(){ $scope.sendCommand('f'); });
  key('s', function(){ $scope.sendCommand('b'); });
  key('a', function(){ $scope.sendCommand('l'); });
  key('d', function(){ $scope.sendCommand('r'); });
  key('1', function(){ $scope.sendCommand('s150'); });
  key('2', function(){ $scope.sendCommand('s200'); });
  key('3', function(){ $scope.sendCommand('s255'); });
}
