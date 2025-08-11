//copied from https://github.com/m0t0k1ch1/ERC721-token-sample/blob/master/

pragma solidity ^0.5.0;

contract velink_mobility {

  uint256 private c_totalSupply = 5000;
  
  /*** CONSTANTS ***/
  address public contract_owner;
  string public constant name = "velink_mobility";

  bytes4 constant InterfaceID_ERC721 =
    bytes4(keccak256('name()')) ^
    bytes4(keccak256('totalSupply()')) ^
    bytes4(keccak256('balanceOf(address)')) ^
    bytes4(keccak256('ownerOf(uint256)')) ^
    bytes4(keccak256('approve(address,uint256)')) ^
    bytes4(keccak256('transfer(address,uint256)')) ^
    bytes4(keccak256('transferFrom(address,address,uint256)')) ^
    bytes4(keccak256('tokensOfOwner(address)'));


  /*** DATA TYPES ***/

  struct UserToken {
    address mintedBy;
    uint64 mintedAt;
    string allowed_vehicle;
  }
  
  struct VehicleToken {
    address mintedBy;
    uint64 mintedAt;
    string vehicle_type;
    string lock_state;
  }


  /*** STORAGE ***/

  UserToken[] user_tokens;
  VehicleToken[] vehicle_tokens;

  mapping (uint256 => address) public tokenIndexToOwner;
  mapping (address => uint256) ownershipTokenCount;
  mapping (uint256 => address) public tokenIndexToApproved;


  constructor() public{
    contract_owner = msg.sender;
  }
  
  /*** EVENTS ***/

  event MintUserToken(address owner, uint256 tokenId);
  event MintVehicleToken(address owner, uint256 tokenId);

// Events
    event Transfer(address from, address to, uint256 tokenId);
    event Approval(address owner, address approved, uint256 tokenId);

  /*** INTERNAL FUNCTIONS ***/
 

  function _owns(address _claimant, uint256 _tokenId) internal view returns (bool) {
    return tokenIndexToOwner[_tokenId] == _claimant;
  }

  function _approvedFor(address _claimant, uint256 _tokenId) internal view returns (bool) {
    return tokenIndexToApproved[_tokenId] == _claimant;
  }

  function _approve(address _to, uint256 _tokenId) internal {
    tokenIndexToApproved[_tokenId] = _to;

    emit Approval(tokenIndexToOwner[_tokenId], tokenIndexToApproved[_tokenId], _tokenId);
  }

  function _transfer(address _from, address _to, uint256 _tokenId) internal {
    ownershipTokenCount[_to]++;
    tokenIndexToOwner[_tokenId] = _to;

    if (_from != address(0)) {
      ownershipTokenCount[_from]--;
      delete tokenIndexToApproved[_tokenId];
    }

    emit Transfer(_from, _to, _tokenId);
  }

  function _mintUserToken(address _owner, string memory _allowed_vehicle) internal returns (uint256 user_token_id) {
    UserToken memory user_token = UserToken({
      mintedBy: _owner,
      mintedAt: uint64(now),
      allowed_vehicle: _allowed_vehicle
    });
    user_token_id = user_tokens.push(user_token) - 1;

    emit MintUserToken(_owner, user_token_id);

    _transfer(address(0), _owner, user_token_id);
  }
  
  function _mintVehicleToken(address _owner, string memory _vehicle_type) internal returns (uint256 vehicle_token_id) {
    VehicleToken memory vehicle_token = VehicleToken({
      mintedBy: _owner,
      mintedAt: uint64(now),
      vehicle_type: _vehicle_type,
      lock_state: "locked"
    });
    vehicle_token_id = vehicle_tokens.push(vehicle_token) - 1;

    emit MintVehicleToken(_owner, vehicle_token_id);

    _transfer(address(0), _owner, vehicle_token_id);
  }


  /*** ERC721 IMPLEMENTATION ***/


  function totalSupplyUserToken() public view returns (uint256) {
    return user_tokens.length; //c_totalSupply;
  }

  function balanceOf(address _owner) public view returns (uint256) {
    return ownershipTokenCount[_owner];
  }

  function ownerOf(uint256 _tokenId) external view returns (address owner) {
    owner = tokenIndexToOwner[_tokenId];

    require(owner != address(0));
  }

  function approve(address _to, uint256 _tokenId) external {
    require(_owns(msg.sender, _tokenId));

    _approve(_to, _tokenId);
  }

  function transfer(address _to, uint256 _tokenId) external {
    require(_to != address(0));
    require(_to != address(this));
    require(_owns(msg.sender, _tokenId));

    _transfer(msg.sender, _to, _tokenId);
  }

  function transferFrom(address _from, address _to, uint256 _tokenId) external {
    require(_to != address(0));
    require(_to != address(this));
    require(_approvedFor(msg.sender, _tokenId));
    require(_owns(_from, _tokenId));

    _transfer(_from, _to, _tokenId);
  }
  
  
  
  function tokensOfOwner(address _owner) internal view returns (uint256[] memory) {
    uint256 balance = balanceOf(_owner);
    
    uint256[] memory result = new uint256[](balance);

    if (balance == 0) {
      return new uint256[](0);
    } else {
     
      uint256 maxTokenId = c_totalSupply;//totalSupply();
      uint256 idx = 0;

      uint256 tokenId;
      for (tokenId = 1; tokenId <= maxTokenId; tokenId++) {
        if (tokenIndexToOwner[tokenId] == _owner) {
          result[idx] = tokenId;
          idx++;
        }
      }
    }

    return result;
  }


  /*** OTHER EXTERNAL FUNCTIONS ***/

  function mintUserToken(string calldata _allowed_vehicle) external returns (uint256) {
    //require(msg.sender == contract_owner);
    require(user_tokens.length < c_totalSupply);
    return _mintUserToken(msg.sender, _allowed_vehicle);
  }
  
  function mintVehicleToken(string calldata _vehicle_type) external returns (uint256) {
    //require(msg.sender == contract_owner);
    require(vehicle_tokens.length < c_totalSupply);
    return _mintVehicleToken(msg.sender, _vehicle_type);
  }

  function getUserToken(uint256 _tokenId) external view returns (address mintedBy, uint64 mintedAt, string memory allowed_vehicle) {
    
    //TOOD: check if this function is called by allowed parties (normal users are not allewed to call this function)
    
    UserToken memory user_token = user_tokens[_tokenId];

    mintedBy = user_token.mintedBy;
    mintedAt = user_token.mintedAt;
    allowed_vehicle = user_token.allowed_vehicle;
  }
  
  function getUserTokenMine(uint256 _tokenId) external view returns (address mintedBy, uint64 mintedAt, string memory allowed_vehicle) {
     
    uint256[] memory user_token_ids = tokensOfOwner(msg.sender);
    
    if(user_token_ids.length == 0) {
        mintedBy = address(0);
        mintedAt = 111;
        allowed_vehicle = "";
    }
    else{
        UserToken memory user_token = user_tokens[user_token_ids[_tokenId]];
        mintedBy = user_token.mintedBy;
        mintedAt = user_token.mintedAt;
        allowed_vehicle = user_token.allowed_vehicle;
    }
    
  }
  
  function getVehicleToken(uint256 _tokenId) external view returns (address mintedBy, uint64 mintedAt, string memory vehicle_type, string memory lock_state) {
    VehicleToken memory vehicle_token = vehicle_tokens[_tokenId];

    mintedBy = vehicle_token.mintedBy;
    mintedAt = vehicle_token.mintedAt;
    vehicle_type = vehicle_token.vehicle_type;
    lock_state = vehicle_token.lock_state;
  }
  
  function checkPermission(address user, address vehicle) external view returns (uint16) {
    if(user == vehicle)
        return 4;
    uint256[] memory user_token_ids = tokensOfOwner(user);
    uint256[] memory vehicle_token_ids = tokensOfOwner(vehicle);
    
    if(user_token_ids.length == 0) 
        return 1;   // user not registered
    
    if(vehicle_token_ids.length == 0)
        return 2; // vehicle not registerd
    
    uint256 user_token_id = user_token_ids[0]; //checking last token
    uint256 vehicle_token_id = vehicle_token_ids[0]; //checking last token
    
    if(keccak256(abi.encodePacked((user_tokens[user_token_id].allowed_vehicle))) == keccak256(abi.encodePacked((vehicle_tokens[vehicle_token_id].vehicle_type))))
        return 0;   // user is allowed to unlock this type of vehicle
    else
        return 3;   // user not allowd to unlock this type of vehicle
  }
  
  function vehicleUnlock(address vehicle) external returns (uint16) {
    require(vehicle != address(0));
    
    uint256[] memory vehicle_token_ids = tokensOfOwner(vehicle);
    
    if(vehicle_token_ids.length == 0)
        return 2; // vehicle not registerd
    
    uint256 vehicle_token_id = vehicle_token_ids[0]; //checking last token
    vehicle_tokens[vehicle_token_id].lock_state = "unlocked";

    _transfer(msg.sender, vehicle, vehicle_token_id);
    return 0;
  }
  
  function vehicleLock(address vehicle) external returns (uint16) {
    require(vehicle != address(0));
    
    uint256[] memory vehicle_token_ids = tokensOfOwner(vehicle);
    
    
    if(vehicle_token_ids.length == 0)
        return 2; // vehicle not registerd
    
    uint256 vehicle_token_id = vehicle_token_ids[0]; //checking last token
    vehicle_tokens[vehicle_token_id].lock_state = "locked";

    _transfer(msg.sender, vehicle, vehicle_token_id);
    return 0;
  }
  
  function getVehicleLockStatus(address vehicle) external view returns (string memory) {
    require(vehicle != address(0));
    
    uint256[] memory vehicle_token_ids = tokensOfOwner(vehicle);
    
    if(vehicle_token_ids.length == 0)
        return "not registered"; // vehicle not registerd
    
    uint256 vehicle_token_id = vehicle_token_ids[0]; //checking last token
    
    return vehicle_tokens[vehicle_token_id].lock_state;
  }
  
}