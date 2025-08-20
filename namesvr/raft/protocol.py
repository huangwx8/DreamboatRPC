from builtins import object

import time

class MessageType(object):
	RequestVotes = 0
	RequestVoteResponse = 1
	Heartbeat = 2
	Acknowledge = 3
	AppendEntries = 4
	Committal = 5
	ClientRequest = 6

class MessageDirection(object):
	Request = 0
	Response = 1	

class RequestVotesResults(object): 
	def __init__(self, term=None, vote_granted=None, message=None):
		if (message is not None):
			self.un_jsonify(message)
		else:
			self._term = term
			self._vote_granted = vote_granted

	@property
	def term(self):
		return self._term

	@property
	def vote_granted(self):
		return self._vote_granted

	def un_jsonify(self, message):
		self._term = 			message['term']     
		self._vote_granted =	message['vote_granted']     

	def jsonify(self):
		return {
			'term':      		self._term,
			'vote_granted': 	self._vote_granted
		}

class AppendEntriesResults(object):
	def __init__(self, term=None, success=None, message=None):
		if (message is not None):
			self.un_jsonify(message)
		else:
			self._term = term
			self._success = success

	@property
	def term(self):
		return self._term

	@property
	def success(self):
		return self._success

	def un_jsonify(self, message):
		self._term = 	message['term']     
		self._success =	message['success']     

	def jsonify(self):
		return {
			'term':      	self._term,
			'success':      self._success
		}

class BaseMessage(object):

	def __init__(self, type_, term, sender, receiver, direction, results):
		self._timestamp = int(time.time())
		self._type = type_
		self._term = term				
		self._sender = sender
		self._receiver = receiver
		self._direction = direction
		self._results = results

	@property
	def timestamp(self):
		return self._timestamp

	@property
	def type(self):
		return self._type

	@property
	def term(self):
		return self._term

	@property
	def sender(self):
		return self._sender

	@property
	def receiver(self):
		return self._receiver

	@property
	def direction(self):
		return self._direction

	@property
	def results(self):
		return self._results

	def un_jsonify(self, message):
		self._type = 			message['type']     
		self._term = 			message['term']     
		self._timestamp = 		message['timestamp']
		self._sender = 			message['sender']
		self._receiver = 		message['receiver'] 
		self._direction = 		message['direction']
		if (self._type == MessageType.RequestVotes):
			self._results = RequestVotesResults(message=message['results'])
		else:
			self._results = AppendEntriesResults(message=message['results'])

	def jsonify(self):
		return {
			'type':      	self._type,
			'term':      	self._term,
			'timestamp': 	self._timestamp,
			'sender':    	self._sender,
			'receiver':  	self._receiver,
			'direction': 	self._direction,
			'results': 		self._results.jsonify()
		}

class RequestVotesMessage(BaseMessage):
	def __init__(self, type_=None, term=None, sender=None, receiver=None, direction=None, results=None, candidate_id=None, last_log_index=None, last_log_term=None, message=None):
		if (message is not None):
			self.un_jsonify(message)
		else:
			if (results is None):
				results = RequestVotesResults()
			BaseMessage.__init__(self, type_, term, sender, receiver, direction, results)
			self._candidate_id = candidate_id
			self._last_log_index = last_log_index
			self._last_log_term = last_log_term
		
	@property
	def candidate_id(self):
		return self._candidate_id

	@property
	def last_log_index(self):
		return self._last_log_index

	@property
	def last_log_term(self):
		return self._last_log_term

	def un_jsonify(self, message):
		BaseMessage.un_jsonify(self, message)
		self._candidate_id = 	message['candidate_id']
		self._last_log_index = 	message['last_log_index']
		self._last_log_term = 	message['last_log_term']

	def jsonify(self):
		message = BaseMessage.jsonify(self)
		message.update({
			'candidate_id': 	self._candidate_id,
			'last_log_index': 	self._last_log_index,
			'last_log_term': 	self._last_log_term
		})
		return message

class AppendEntriesMessage(BaseMessage):
	def __init__(self, type_=None, term=None, sender=None, receiver=None, direction=None, results=None, leader_id=None, prev_log_index=None, prev_log_term=None, entries=None, leader_commit=None, message=None):
		if (message is not None):
			self.un_jsonify(message)
		else:
			if (results is None):
				results = AppendEntriesResults()
			BaseMessage.__init__(self, type_, term, sender, receiver, direction, results)
			self._leader_id = leader_id
			self._prev_log_index = prev_log_index
			self._prev_log_term = prev_log_term
			self._entries = entries
			self._leader_commit = leader_commit

	@property
	def leader_id(self):
		return self._leader_id

	@property
	def prev_log_index(self):
		return self._prev_log_index

	@property
	def prev_log_term(self):
		return self._prev_log_term

	@property
	def entries(self):
		return self._entries

	@property
	def leader_commit(self):
		return self._leader_commit

	def un_jsonify(self, message):
		BaseMessage.un_jsonify(self, message)
		self._leader_id = 		message['leader_id']
		self._prev_log_index = 	message['prev_log_index']
		self._prev_log_term = 	message['prev_log_term']
		self._entries = 			message['entries']
		self._leader_commit = 	message['leader_commit']

	def jsonify(self):
		message = BaseMessage.jsonify(self)
		message.update({
			'leader_id': 		self._leader_id,
			'prev_log_index': 	self._prev_log_index,
			'prev_log_term': 	self._prev_log_term,
			'entries': 			self._entries,
			'leader_commit': 	self._leader_commit
		})
		return message

def parse_json_message(json_message):
	if json_message is None:
		return None
	elif json_message['type'] == MessageType.RequestVotes:
		return RequestVotesMessage(message=json_message)
	else:
		return AppendEntriesMessage(message=json_message)
